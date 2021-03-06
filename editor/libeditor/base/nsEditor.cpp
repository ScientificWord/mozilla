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
 *   Pierre Phaneuf <pp@ludusdesign.com>
 *   Daniel Glazman <glazman@netscape.com>
 *   Masayuki Nakano <masayuki@d-toybox.com>
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

#include "pratom.h"
#include "prprf.h"
#include "nsIDOMDocument.h"
#include "nsIDOMXMLDocument.h"
#include "nsIDOMHTMLDocument.h"
#include "nsIDOMHTMLElement.h"
#include "nsIDOMNSHTMLElement.h"
#include "nsIDOMEventTarget.h"
#include "nsPIDOMEventTarget.h"
#include "nsIEventListenerManager.h"
#include "nsIPrefBranch.h"
#include "nsIPrefService.h"
#include "nsUnicharUtils.h"
#include "nsReadableUtils.h"

#include "nsIDOMText.h"
#include "nsIDOMElement.h"
#include "nsIDOMAttr.h"
#include "nsIDOMNode.h"
#include "nsIDOM3Node.h"
#include "nsIDOMDocumentFragment.h"
#include "nsIDOMNamedNodeMap.h"
#include "nsIDOMNodeList.h"
#include "nsIDOMRange.h"
#include "nsIDOMEventListener.h"
#include "nsIDOMEventGroup.h"
#include "nsIDOMMouseListener.h"
#include "nsIDOMFocusListener.h"
#include "nsIDOMTextListener.h"
#include "nsIDOMCompositionListener.h"
#include "nsIDOMDragListener.h"
#include "nsIDOMHTMLBRElement.h"
#include "nsIDocument.h"
#include "nsITransactionManager.h"
#include "nsIAbsorbingTransaction.h"
#include "nsIPresShell.h"
#include "nsIViewManager.h"
#include "nsISelection.h"
#include "nsISelectionPrivate.h"
#include "nsISelectionController.h"
#include "nsIEnumerator.h"
#include "nsIAtom.h"
#include "nsICaret.h"
#include "nsIKBStateControl.h"
#include "nsIWidget.h"
#include "nsIPlaintextEditor.h"
#include "nsIHTMLEditor.h"
#include "../html/nsHTMLEditor.h"
#include "nsTextEditUtils.h"
#include "nsGUIEvent.h"  // nsTextEventReply

#include "nsIFrame.h"  // Needed by IME code

#include "nsICSSStyleSheet.h"

#include "nsIContent.h"
#include "nsServiceManagerUtils.h"

// transactions the editor knows how to build
#include "TransactionFactory.h"
#include "EditAggregateTxn.h"
#include "PlaceholderTxn.h"
#include "ChangeAttributeTxn.h"
#include "CreateElementTxn.h"
#include "InsertElementTxn.h"
#include "DeleteElementTxn.h"
#include "ReplaceElementTxn.h"
#include "InsertTextTxn.h"
#include "DeleteTextTxn.h"
#include "DeleteRangeTxn.h"
#include "SplitElementTxn.h"
#include "JoinElementTxn.h"
#include "nsStyleSheetTxns.h"
#include "IMETextTxn.h"

#include "nsEditor.h"
#include "nsEditorUtils.h"
#include "nsISelectionDisplay.h"
#include "nsIInlineSpellChecker.h"
#include "nsINameSpaceManager.h"
#include "nsIHTMLDocument.h"
#include "nsIParserService.h"
#include "nsHTMLEditUtils.h"
 #include "msiUtils.h"
//ljh
#include "msiIEditActionListenerExtension.h"

#define NS_ERROR_EDITOR_NO_SELECTION NS_ERROR_GENERATE_FAILURE(NS_ERROR_MODULE_EDITOR,1)
#define NS_ERROR_EDITOR_NO_TEXTNODE  NS_ERROR_GENERATE_FAILURE(NS_ERROR_MODULE_EDITOR,2)

#ifdef NS_DEBUG_EDITOR
static PRBool gNoisy = PR_FALSE;
#endif


extern "C" {
void DumpNode(nsIDOMNode *aNode, PRInt32 indent, bool recurse, nsAString& output)
{
#ifdef DEBUG
  PRInt32 i;
  nsAutoString tag;
  nsAutoString nodeName;
  nsAutoString tagWithAttributes;
  nsString oneIndent = NS_LITERAL_STRING("  ");
  nsString newline = NS_LITERAL_STRING("\n");
  nsString indentString = newline;

  for (i=0; i<indent; i++) {
    indentString.Append(oneIndent);
  }
  output.Append(indentString);

  if (aNode == 0){
  #ifdef debug_barry
    printf("!NULL\n");
  #endif
    output = NS_LITERAL_STRING("!NULL!");
    return;
  }

  PRUint16 nodeType = 0;
  aNode->GetNodeType(&nodeType);

  nsCOMPtr<nsIDOMElement> element = do_QueryInterface(aNode);
  nsCOMPtr<nsIDOMDocumentFragment> docfrag = do_QueryInterface(aNode);

  if ((nodeType == 1 /* element */) && element)
  {
    element->GetTagName(tag);
    nsEditor::DumpTagName(element, tagWithAttributes);
    // output.Append(indentString);
    output.Append(NS_LITERAL_STRING("<") + tagWithAttributes);
    output.Append(NS_LITERAL_STRING(">"));
  }
  else if ((nodeType == 11 /* Document fragment */) && docfrag)
  {
    tag = NS_LITERAL_STRING("document fragment");
    // output.Append(indentString);
    output.Append(NS_LITERAL_STRING("<document fragment>"));//, ((nsIDOMDocumentFragment*)docfrag)-28));
  }
  else if ((nodeType == 3 /* text */ )) {
    aNode->GetNodeName(nodeName);
    if (nodeName.EqualsLiteral("#text"))
    {
      nsCOMPtr<nsIDOMCharacterData> textNode = do_QueryInterface(aNode);
      nsAutoString str;
      textNode->GetData(str);
      // output.Append(indentString);
      output.Append(NS_LITERAL_STRING("<#text '") + str + NS_LITERAL_STRING("'>"));
    }
  }

  if (recurse){
     nsCOMPtr<nsIDOMNodeList> childList;
     aNode->GetChildNodes(getter_AddRefs(childList));
     if (!childList) return; // NS_ERROR_NULL_POINTER;
     PRUint32 numChildren;
     childList->GetLength(&numChildren);
     nsCOMPtr<nsIDOMNode> child, tmp;
     aNode->GetFirstChild(getter_AddRefs(child));
     for (i=0; i<numChildren; i++)
     {
       DumpNode(child, indent+1, true, output);
       child->GetNextSibling(getter_AddRefs(tmp));
       child = tmp;
     }
  }
  if (nodeType != 3 ) {
    output.Append(indentString);
    output.Append(NS_LITERAL_STRING("</") + tag + NS_LITERAL_STRING(">"));
  }
  if (indent == 0) 
    output.Append(newline);
#endif
}

// void DumpFrame(nsIFrame *aFrame, PRInt32 indent, bool recurse, nsAString& output)
// {
// #ifdef DEBUG
//   PRInt32 i;
//   nsAutoString tag;
//   nsAutoString nodeName;
//   nsAutoString tagWithAttributes;
//   nsString oneIndent = NS_LITERAL_STRING("  ");
//   nsString newline = NS_LITERAL_STRING("\n");
//   nsString indentString = newline;

//   for (i=0; i<indent; i++) {
//     indentString.Append(oneIndent);
//   }
//   output.Append(indentString);

//   if (aFrame == 0){
//     printf("!NULL\n");
//     output = NS_LITERAL_STRING("!NULL!");
//     return;
//   }

//   PRUint16 nodeType = 0;
//   aNode->GetNodeType(&nodeType);

//   nsCOMPtr<nsIDOMElement> element = do_QueryInterface(aNode);
//   nsCOMPtr<nsIDOMDocumentFragment> docfrag = do_QueryInterface(aNode);

//   if ((nodeType == 1 /* element */) && element)
//   {
//     element->GetTagName(tag);
//     nsEditor::DumpTagName(element, tagWithAttributes);
//     // output.Append(indentString);
//     output.Append(NS_LITERAL_STRING("<") + tagWithAttributes);
//     output.Append(NS_LITERAL_STRING(">"));
//   }
//   else if ((nodeType == 11 /* Document fragment */) && docfrag)
//   {
//     tag = NS_LITERAL_STRING("document fragment");
//     // output.Append(indentString);
//     output.Append(NS_LITERAL_STRING("<document fragment>"));//, ((nsIDOMDocumentFragment*)docfrag)-28));
//   }
//   else if ((nodeType == 3 /* text */ )) {
//     aNode->GetNodeName(nodeName);
//     if (nodeName.EqualsLiteral("#text"))
//     {
//       nsCOMPtr<nsIDOMCharacterData> textNode = do_QueryInterface(aNode);
//       nsAutoString str;
//       textNode->GetData(str);
//       // output.Append(indentString);
//       output.Append(NS_LITERAL_STRING("<#text '") + str + NS_LITERAL_STRING("'>"));
//     }
//   }

//   if (recurse){
//      nsCOMPtr<nsIDOMNodeList> childList;
//      aNode->GetChildNodes(getter_AddRefs(childList));
//      if (!childList) return; // NS_ERROR_NULL_POINTER;
//      PRUint32 numChildren;
//      childList->GetLength(&numChildren);
//      nsCOMPtr<nsIDOMNode> child, tmp;
//      aNode->GetFirstChild(getter_AddRefs(child));
//      for (i=0; i<numChildren; i++)
//      {
//        DumpNode(child, indent+1, true, output);
//        child->GetNextSibling(getter_AddRefs(tmp));
//        child = tmp;
//      }
//   }
//   if (nodeType != 3 ) {
//     output.Append(indentString);
//     output.Append(NS_LITERAL_STRING("</") + tag + NS_LITERAL_STRING(">"));
//   }
//   if (indent == 0) 
//     output.Append(newline);
// #endif
// }

void AppendInt(nsAString &str, PRInt32 val)
{
  char buf[32];
  PR_snprintf(buf, sizeof(buf), "%ld", val);
  str.Append(NS_ConvertASCIItoUTF16(buf));
}

void DumpRange( nsIDOMRange * range, PRInt32 indent, nsAString& output) {
#ifdef DEBUG
  nsCOMPtr<nsIDOMNode> anchorNode, focusNode;
  PRInt32 anchorOffset, focusOffset;
  PRBool collapsed;
  PRBool sameNode;
  nsString endline = NS_LITERAL_STRING("\n");
  nsString anchorStr = NS_LITERAL_STRING("\nANCHOR: offset=");
  nsString focusStr = NS_LITERAL_STRING("\nFOCUS: offset=");
  nsString collapsedStr = NS_LITERAL_STRING("\nANCHOR=FOCUS: offset=");
  range->GetStartContainer(getter_AddRefs(anchorNode));
  range->GetStartOffset(&anchorOffset);
  range->GetEndContainer(getter_AddRefs(focusNode));
  range->GetEndOffset(&focusOffset);
  sameNode = (anchorNode == focusNode);
  range->GetCollapsed(&collapsed);
  if (collapsed) {
   output.Append(collapsedStr);
   AppendInt(output, focusOffset);
  }
  else if (sameNode) {
   output.Append(anchorStr);
   AppendInt(output, anchorOffset);
   output.Append(focusStr);
   AppendInt(output, focusOffset);
  }
  else {
   output.Append(anchorStr);
   AppendInt(output, anchorOffset);
   DumpNode(anchorNode, indent, PR_TRUE, output);
   output += focusStr;
   AppendInt(output, focusOffset);
  }
  DumpNode(focusNode, indent, PR_TRUE, output);
#endif //Debug
}


void DumpSelection( nsISelection * sel, PRInt32 indent, nsAString& output) {
#ifdef DEBUG
  nsCOMPtr<nsIDOMRange> range;
  sel->GetRangeAt(0, getter_AddRefs(range));
  DumpRange( range, indent, output);
#endif //Debug
}

} // extern C

#ifdef DEBUG_Barry
void DumpDocumentNodeImpl( nsIDOMNode * pNode, PRUint32 indent)
{
  nsCOMPtr<nsIDOMNodeList> list;
  if (!pNode) return;
  nsresult res;
  nsCOMPtr<nsIDOMNode> node;
  nsAutoString name;
  res = pNode->GetChildNodes(getter_AddRefs(list));
  PRInt32 i;
  PRInt32 j;
  PRUint32 childCount=0;
  list->GetLength(&childCount);
  for (i = 0; i < childCount; i++)
  {
    res = list->Item(i, getter_AddRefs(node));
    res = node->GetNodeName(name);
    for (j = 0; j<indent; j++) printf("  ");
    printf("%d %S (%x)\n", i, name.get(), node);
    DumpDocumentNodeImpl(node, indent + 1);
  }
}


void DumpDocumentNode( nsIDOMNode * pNode)
{
//  DumpDocumentNodeImpl(pNode, 0);
}
#endif //DEBUG_Barry

// Defined in nsEditorRegistration.cpp
extern nsIParserService *sParserService;

//---------------------------------------------------------------------------
//
// nsEditor: base editor class implementation
//
//---------------------------------------------------------------------------

nsIAtom *nsEditor::gTypingTxnName;
nsIAtom *nsEditor::gIMETxnName;
nsIAtom *nsEditor::gDeleteTxnName;

nsEditor::nsEditor()
:  mModCount(0)
,  mPresShellWeak(nsnull)
,  mViewManager(nsnull)
,  mUpdateCount(0)
,  mSpellcheckCheckboxState(eTriUnset)
,  mPlaceHolderTxn(nsnull)
,  mPlaceHolderName(nsnull)
,  mPlaceHolderBatch(0)
,  mSelState(nsnull)
,  mSavedSel()
,  mRangeUpdater()
,  mAction(nsnull)
,  mDirection(eNone)
,  mIMETextNode(nsnull)
,  mIMETextOffset(0)
,  mIMEBufferLength(0)
,  mInIMEMode(PR_FALSE)
,  mIsIMEComposing(PR_FALSE)
,  mShouldTxnSetSelection(PR_TRUE)
,  mDidPreDestroy(PR_FALSE)
,  mDocDirtyState(-1)
,  mDocWeak(nsnull)
,  mPhonetic(nsnull)
,  ctrlDown(PR_FALSE)
,  metaDown(PR_FALSE)
,  altDown(PR_FALSE)
,  shiftDown(PR_FALSE)
{
  //initialize member variables here
  if (!gTypingTxnName)
    gTypingTxnName = NS_NewAtom("Typing");
  else
    NS_ADDREF(gTypingTxnName);
  if (!gIMETxnName)
    gIMETxnName = NS_NewAtom("IME");
  else
    NS_ADDREF(gIMETxnName);
  if (!gDeleteTxnName)
    gDeleteTxnName = NS_NewAtom("Deleting");
  else
    NS_ADDREF(gDeleteTxnName);
}

nsEditor::~nsEditor()
{
  /* first, delete the transaction manager if there is one.
     this will release any remaining transactions.
     this is important because transactions can hold onto the atoms (gTypingTxnName, ...)
     and to make the optimization (holding refcounted statics) work correctly,
     the editor instance needs to hold the last refcount.
     If you get this wrong, expect to deref a garbage gTypingTxnName pointer if you bring up a second editor.
  */
  if (mTxnMgr) {
    mTxnMgr = 0;
  }
  nsrefcnt refCount=0;
  if (gTypingTxnName)  // we addref'd in the constructor
  { // want to release it without nulling out the pointer.
    refCount = gTypingTxnName->Release();
    if (0==refCount) {
      gTypingTxnName = nsnull;
    }
  }

  if (gIMETxnName)  // we addref'd in the constructor
  { // want to release it without nulling out the pointer.
    refCount = gIMETxnName->Release();
    if (0==refCount) {
      gIMETxnName = nsnull;
    }
  }

  if (gDeleteTxnName)  // we addref'd in the constructor
  { // want to release it without nulling out the pointer.
    refCount = gDeleteTxnName->Release();
    if (0==refCount) {
      gDeleteTxnName = nsnull;
    }
  }

  delete mPhonetic;

  NS_IF_RELEASE(mViewManager);
}

NS_IMPL_ISUPPORTS5(nsEditor, nsIEditor, nsIEditorIMESupport,
                   nsISupportsWeakReference, nsIPhonetic, nsIMutationObserver)


//#ifdef DEBUG_barry

void
nsEditor::DumpTagName(nsIDOMNode *aNode, nsAString & nodeInfo)
{
  nsCOMPtr<nsIDOMElement> element = do_QueryInterface(aNode);
  nsCOMPtr<nsIDOMAttr> item;
  nsCOMPtr<nsIDOMNode> itemNode;
  nsAutoString tag;
  nsAutoString name, value;
  NS_NAMED_LITERAL_STRING(quote,"\"");
  NS_NAMED_LITERAL_STRING(equals,"=");
  NS_NAMED_LITERAL_STRING(space," ");
  PRUint32 length, i;
  element->GetTagName(nodeInfo);
  nsCOMPtr<nsIDOMNamedNodeMap> map;
  element->GetAttributes(getter_AddRefs(map));
  map->GetLength(&length);
  for ( i = 0; i < length; i++)
  {
    map->Item(i, getter_AddRefs(itemNode));
    if (itemNode) {
      item = do_QueryInterface(itemNode);
      item -> GetName(name);
      item -> GetValue(value);
      nodeInfo += space + name + equals + quote + value + quote;
    }
  }
  // printf("%s %s\n",NS_LossyConvertUTF16toASCII(tag).get(),NS_LossyConvertUTF16toASCII(attributes).get());
}


#if 0
nsString
nsEditor::DumpNodeS(nsIDOMNode *aNode, PRInt32 indent, bool recurse /* = false */)
{
  PRInt32 i;
  nsAutoString sret;
  nsAutoString tag;
  nsAutoString tagWithAttributes;
  for (i=0; i<indent; i++)
    sret.Append("  ");

  if (aNode == 0){
    sret.Append("!NULL!\n");
	  return sret;
  }

  nsCOMPtr<nsIDOMElement> element = do_QueryInterface(aNode);
  nsCOMPtr<nsIDOMDocumentFragment> docfrag = do_QueryInterface(aNode);

  if (element || docfrag)
  {
    if (element)
    {
      element->GetTagName(tag);
      DumpTagName(element, tagWithAttributes);
      sret.Append("<%s>    0x%x\n", NS_LossyConvertUTF16toASCII(tagWithAttributes).get(), ((nsIDOMElement*)element)-28);
    }
    else
    {
      sret.Append("<document fragment>       0x%x\n", ((nsIDOMDocumentFragment*)docfrag)-28);
    }
	  if (recurse){
       nsCOMPtr<nsIDOMNodeList> childList;
       aNode->GetChildNodes(getter_AddRefs(childList));
       if (!childList) return sret; // NS_ERROR_NULL_POINTER;
       PRUint32 numChildren;
       childList->GetLength(&numChildren);
       nsCOMPtr<nsIDOMNode> child, tmp;
       aNode->GetFirstChild(getter_AddRefs(child));
       for (i=0; i<numChildren; i++)
       {
         DumpNode(child, indent+1, true);
         child->GetNextSibling(getter_AddRefs(tmp));
         child = tmp;
       }
	  }
    for (i=0; i<indent; i++) {
      sret.Append("  ");
    }
    sret.Append("</%s>\n", NS_LossyConvertUTF16toASCII(tag).get());
  }
  else if (IsTextNode(aNode))
  {
    nsCOMPtr<nsIDOMCharacterData> textNode = do_QueryInterface(aNode);
    nsAutoString str;
    textNode->GetData(str);
    nsCAutoString cstr;
    LossyCopyUTF16toASCII(str, cstr);
    // cstr.ReplaceChar('\n', ' ');
    sret.Append("#text \'%s\'     0x%x\n", cstr.get(), aNode-28);
  }
  return sret;
}
#endif


/** returns PR_TRUE if aNode is of the type implied by aTag */
PRBool nsEditor::NodeIsType(nsIDOMNode *aNode, nsIAtom *aTag, msiITagListManager * manager)
{
  nsIAtom * nodeAtom = nsEditor::GetTag(aNode);
	if (!nodeAtom) return PR_FALSE;
  if (nodeAtom == aTag) return PR_TRUE;
  PRBool fIsInTagClass;
  if (manager)
  {
    nsAutoString strNodeName;
    nsAutoString strTagName;
    nodeAtom->ToString(strNodeName);
    aTag->ToString(strTagName);
    manager->GetTagInClass(strTagName, strNodeName, nsnull, &fIsInTagClass);
    if (fIsInTagClass) return PR_TRUE;
  }
  return PR_FALSE;
}

// we should get rid of this method if we can
PRBool nsEditor::NodeIsTypeString(nsIDOMNode *aNode, const nsAString &aTag, msiITagListManager * manager)
{
  nsIAtom *nodeAtom = nsEditor::GetTag(aNode);
  if (nodeAtom && nodeAtom->Equals(aTag)) return PR_TRUE;
  PRBool fIsInTagClass;
  if (manager) {
    nsAutoString strNodeName;
    nsAutoString strClassName;
    nodeAtom->ToString(strNodeName);
    manager->GetRealClassOfTag(strNodeName, nsnull, strClassName);
    fIsInTagClass = aTag.Equals(strClassName);
//    manager->GetTagInClass(aTag, strNodeName, nsnull, &fIsInTagClass);
    if (fIsInTagClass) return PR_TRUE;
  }
  return PR_FALSE;
}



NS_IMETHODIMP
nsEditor::Init(nsIDOMDocument *aDoc, nsIPresShell* aPresShell, nsIContent *aRoot, nsISelectionController *aSelCon, PRUint32 aFlags)
{
  NS_PRECONDITION(nsnull!=aDoc && nsnull!=aPresShell, "bad arg");
  if ((nsnull==aDoc) || (nsnull==aPresShell))
    return NS_ERROR_NULL_POINTER;

  mFlags = aFlags;
  mDocWeak = do_GetWeakReference(aDoc);  // weak reference to doc
  mPresShellWeak = do_GetWeakReference(aPresShell);   // weak reference to pres shell
  mSelConWeak = do_GetWeakReference(aSelCon);   // weak reference to selectioncontroller
  nsCOMPtr<nsIPresShell> ps = do_QueryReferent(mPresShellWeak);
  if (!ps) return NS_ERROR_NOT_INITIALIZED;

  //set up root element if we are passed one.
  if (aRoot)
    mRootElement = do_QueryInterface(aRoot);

  nsCOMPtr<nsINode> document = do_QueryInterface(aDoc);
  document->AddMutationObserver(this);

  // Set up the DTD
  // XXX - in the long run we want to get this from the document, but there
  // is no way to do that right now.  So we leave it null here and set
  // up a nav html dtd in nsHTMLEditor::Init

  mViewManager = ps->GetViewManager();
  if (!mViewManager) {return NS_ERROR_NULL_POINTER;}
  NS_ADDREF(mViewManager);

  mUpdateCount=0;

  /* initialize IME stuff */
  mIMETextNode = nsnull;
  mIMETextOffset = 0;
  mIMEBufferLength = 0;

  /* Show the caret */
  aSelCon->SetCaretReadOnly(PR_FALSE);
  aSelCon->SetDisplaySelection(nsISelectionController::SELECTION_ON);

  aSelCon->SetSelectionFlags(nsISelectionDisplay::DISPLAY_ALL);//we want to see all the selection reflected to user

#if 1
  // THIS BLOCK CAUSES ASSERTIONS because sometimes we don't yet have
  // a moz-br but we do have a presshell.

  // Set the selection to the beginning:

//hack to get around this for now.
  nsCOMPtr<nsIPresShell> shell = do_QueryReferent(mSelConWeak);
  if (shell)
    BeginningOfDocument();
#endif

  NS_POSTCONDITION(mDocWeak && mPresShellWeak, "bad state");

  return NS_OK;
}


///////////////////////////////////////////////////////////////////////////
// RemoveContainer: remove inNode, reparenting its children into their
//                  the parent of inNode
//

/* void removeContainer (in nsIDOMNode node); */
NS_IMETHODIMP
 nsEditor::RemoveContainer(nsIDOMNode *inNode)
{
  if (!inNode)
    return NS_ERROR_NULL_POINTER;
  nsCOMPtr<nsIDOMNode> parent;
  PRInt32 offset;

  nsresult res = GetNodeLocation(inNode, address_of(parent), &offset);
  if (NS_FAILED(res)) return res;

  // loop through the child nodes of inNode and promote them
  // into inNode's parent.
  PRBool bHasMoreChildren;
  inNode->HasChildNodes(&bHasMoreChildren);
  nsCOMPtr<nsIDOMNodeList> nodeList;
  res = inNode->GetChildNodes(getter_AddRefs(nodeList));
  if (NS_FAILED(res)) return res;
  if (!nodeList) return NS_ERROR_NULL_POINTER;
  PRUint32 nodeOrigLen;
  nodeList->GetLength(&nodeOrigLen);

  // notify our internal selection state listener
  nsAutoRemoveContainerSelNotify selNotify(mRangeUpdater, inNode, parent, offset, nodeOrigLen);

  {
    nsAutoTxnsConserveSelection conserveSelection(this);

    // Move all children from inNode to its parent.
    inNode->HasChildNodes(&bHasMoreChildren);
    // PRBool inComplexTransaction;
    // isInComplexTransaction(PR_TRUE, &inComplexTransaction);

    while (bHasMoreChildren) {
      nsCOMPtr<nsIDOMNode> child;
      res = inNode->GetLastChild(getter_AddRefs(child));
      res = DeleteNode(child);
      if (NS_FAILED(res)) {
        return res;
      }

      // Insert the last child before the previous last child.  So, we need to
      // use offset here because previous child might have been moved to
      // container.
      res = InsertNode(child,
                      parent, offset);

      if (NS_FAILED(res)) {
        return res;
      }
      inNode->HasChildNodes(&bHasMoreChildren);

    }
    // IsInComplexTransaction(inComplexTransaction, nsnull);
    DeleteNode(inNode);
  }
}



/* attribute boolean inComplexTransaction; */
NS_IMETHODIMP
nsEditor::IsInComplexTransaction(PRBool newValue, PRBool *aInComplexTransaction)
{
  if (aInComplexTransaction) 
    *aInComplexTransaction = isInComplexTransaction;
  isInComplexTransaction = newValue;
  return NS_OK;
}

NS_IMETHODIMP
nsEditor::ReadInComplexTransaction(PRBool *aInComplexTransaction)
{
  if (aInComplexTransaction) 
    *aInComplexTransaction = isInComplexTransaction;
  return NS_OK;
}

NS_IMETHODIMP
nsEditor::PostCreate()
{
  // Set up spellchecking
  nsresult rv = SyncRealTimeSpell();
  NS_ENSURE_SUCCESS(rv, rv);

  // Set up listeners
  rv = CreateEventListeners();
  if (NS_FAILED(rv))
  {
    RemoveEventListeners();

    return rv;
  }

  rv = InstallEventListeners();
  NS_ENSURE_SUCCESS(rv, rv);

  // nuke the modification count, so the doc appears unmodified
  // do this before we notify listeners
  ResetModificationCount();

  // update the UI with our state
  NotifyDocumentListeners(eDocumentCreated);
  NotifyDocumentListeners(eDocumentStateChanged);

  return NS_OK;
}

nsresult
nsEditor::InstallEventListeners()
{
  NS_ENSURE_TRUE(mDocWeak && mPresShellWeak && mKeyListenerP &&
                 mMouseListenerP && mFocusListenerP && mTextListenerP &&
                 mCompositionListenerP && mDragListenerP,
                 NS_ERROR_NOT_INITIALIZED);

  nsCOMPtr<nsPIDOMEventTarget> piTarget = GetPIDOMEventTarget();

  if (!piTarget) {
    RemoveEventListeners();
    return NS_ERROR_FAILURE;
  }

  nsresult rv = NS_OK;

  // register the event listeners with the listener manager
  nsCOMPtr<nsIDOMEventGroup> sysGroup;
  piTarget->GetSystemEventGroup(getter_AddRefs(sysGroup));
  nsCOMPtr<nsIEventListenerManager> elmP;
  piTarget->GetListenerManager(PR_TRUE, getter_AddRefs(elmP));

  if (sysGroup && elmP)
  {
    rv = elmP->AddEventListenerByType(mKeyListenerP,
                                      NS_LITERAL_STRING("keypress"),
                                      NS_EVENT_FLAG_BUBBLE |
                                      NS_PRIV_EVENT_UNTRUSTED_PERMITTED,
                                      sysGroup);
    NS_ASSERTION(NS_SUCCEEDED(rv),
                 "failed to register key listener in system group");
  }

  rv |= piTarget->AddEventListenerByIID(mMouseListenerP,
                                        NS_GET_IID(nsIDOMMouseListener));

  if (elmP) {
    // Focus event doesn't bubble so adding the listener to capturing phase.
    // Make sure this works after bug 235441 gets fixed.
    rv |= elmP->AddEventListenerByIID(mFocusListenerP,
                                      NS_GET_IID(nsIDOMFocusListener),
                                      NS_EVENT_FLAG_CAPTURE);
  }

  rv |= piTarget->AddEventListenerByIID(mTextListenerP,
                                        NS_GET_IID(nsIDOMTextListener));

  rv |= piTarget->AddEventListenerByIID(mCompositionListenerP,
                                        NS_GET_IID(nsIDOMCompositionListener));

  rv |= piTarget->AddEventListenerByIID(mDragListenerP,
                                        NS_GET_IID(nsIDOMDragListener));

  if (NS_FAILED(rv))
  {
    NS_ERROR("failed to register some event listeners");

    RemoveEventListeners();
  }

  return rv;
}

void
nsEditor::RemoveEventListeners()
{
  if (!mDocWeak)
  {
    return;
  }

  nsCOMPtr<nsPIDOMEventTarget> piTarget = GetPIDOMEventTarget();

  if (piTarget)
  {
    // unregister the event listeners with the DOM event target
    nsCOMPtr<nsIEventListenerManager> elmP;
    piTarget->GetListenerManager(PR_TRUE, getter_AddRefs(elmP));
    if (mKeyListenerP)
    {
      nsCOMPtr<nsIDOMEventGroup> sysGroup;
      piTarget->GetSystemEventGroup(getter_AddRefs(sysGroup));
      if (sysGroup && elmP)
      {
        elmP->RemoveEventListenerByType(mKeyListenerP,
                                        NS_LITERAL_STRING("keypress"),
                                        NS_EVENT_FLAG_BUBBLE |
                                        NS_PRIV_EVENT_UNTRUSTED_PERMITTED,
                                        sysGroup);
      }
    }

    if (mMouseListenerP)
    {
      piTarget->RemoveEventListenerByIID(mMouseListenerP,
                                         NS_GET_IID(nsIDOMMouseListener));
    }

    if (mFocusListenerP && elmP)
    {
      elmP->RemoveEventListenerByIID(mFocusListenerP,
                                     NS_GET_IID(nsIDOMFocusListener),
                                     NS_EVENT_FLAG_CAPTURE);
    }

    if (mTextListenerP)
    {
      piTarget->RemoveEventListenerByIID(mTextListenerP,
                                         NS_GET_IID(nsIDOMTextListener));
    }

    if (mCompositionListenerP)
    {
      piTarget->RemoveEventListenerByIID(mCompositionListenerP,
                                         NS_GET_IID(nsIDOMCompositionListener));
    }

    if (mDragListenerP)
    {
      piTarget->RemoveEventListenerByIID(mDragListenerP,
                                         NS_GET_IID(nsIDOMDragListener));
    }
  }
}

PRBool
nsEditor::GetDesiredSpellCheckState()
{
  // Check user override on this element
  if (mSpellcheckCheckboxState != eTriUnset) {
    return (mSpellcheckCheckboxState == eTriTrue);
  }

  // Check user preferences
  nsresult rv;
  nsCOMPtr<nsIPrefBranch> prefBranch =
    do_GetService(NS_PREFSERVICE_CONTRACTID, &rv);
  PRInt32 spellcheckLevel = 1;
  if (NS_SUCCEEDED(rv) && prefBranch) {
    prefBranch->GetIntPref("layout.spellcheckDefault", &spellcheckLevel);
  }

  if (spellcheckLevel == 0) {
    return PR_FALSE;                    // Spellchecking forced off globally
  }

  // Check for password/readonly/disabled, which are not spellchecked
  // regardless of DOM
  PRUint32 flags;
  if (NS_SUCCEEDED(GetFlags(&flags)) &&
      flags & (nsIPlaintextEditor::eEditorPasswordMask |
               nsIPlaintextEditor::eEditorReadonlyMask |
               nsIPlaintextEditor::eEditorDisabledMask)) {
    return PR_FALSE;
  }

  nsCOMPtr<nsIPresShell> presShell;
  rv = GetPresShell(getter_AddRefs(presShell));
  if (NS_SUCCEEDED(rv)) {
    nsPresContext* context = presShell->GetPresContext();
    if (context && !context->IsDynamic()) {
        return PR_FALSE;
    }
  }

  // Check DOM state
  nsCOMPtr<nsIContent> content = do_QueryInterface(GetRoot());
  if (!content) {
    return PR_FALSE;
  }

  if (content->IsNativeAnonymous()) {
    content = content->GetParent();
  }

  nsCOMPtr<nsIDOMNSHTMLElement> element = do_QueryInterface(content);
  if (!element) {
    return PR_FALSE;
  }

  PRBool enable;
  element->GetSpellcheck(&enable);

  return enable;
}

NS_IMETHODIMP
nsEditor::PreDestroy()
{
  if (mDidPreDestroy)
    return NS_OK;

  // Let spellchecker clean up its observers etc. It is important not to
  // actually free the spellchecker here, since the spellchecker could have
  // caused flush notifications, which could have gotten here if a textbox
  // is being removed. Setting the spellchecker to NULL could free the
  // object that is still in use! It will be freed when the editor is
  // destroyed.
  if (mInlineSpellChecker)
    mInlineSpellChecker->Cleanup();

  // tell our listeners that the doc is going away
  NotifyDocumentListeners(eDocumentToBeDestroyed);

  nsCOMPtr<nsINode> document = do_QueryReferent(mDocWeak);
  if (document)
    document->RemoveMutationObserver(this);

  // Unregister event listeners
  RemoveEventListeners();
  mActionListeners.Clear();
  mEditorObservers.Clear();
  mDocStateListeners.Clear();
  mInlineSpellChecker = nsnull;

  mDidPreDestroy = PR_TRUE;
  return NS_OK;
}

NS_IMETHODIMP
nsEditor::GetFlags(PRUint32 *aFlags)
{
  *aFlags = mFlags;
  return NS_OK;
}

NS_IMETHODIMP
nsEditor::SetFlags(PRUint32 aFlags)
{
  mFlags = aFlags;

  // Changing the flags can change whether spellchecking is on, so re-sync it
  SyncRealTimeSpell();
  return NS_OK;
}

NS_IMETHODIMP
nsEditor::GetIsDocumentEditable(PRBool *aIsDocumentEditable)
{
  NS_ENSURE_ARG_POINTER(aIsDocumentEditable);
  nsCOMPtr<nsIDOMDocument> doc;
  GetDocument(getter_AddRefs(doc));
  *aIsDocumentEditable = doc ? PR_TRUE : PR_FALSE;

  return NS_OK;
}

NS_IMETHODIMP
nsEditor::GetDocument(nsIDOMDocument **aDoc)
{
  if (!aDoc)
    return NS_ERROR_NULL_POINTER;
  *aDoc = nsnull; // init out param
  NS_PRECONDITION(mDocWeak, "bad state, mDocWeak weak pointer not initialized");
  nsCOMPtr<nsIDOMDocument> doc = do_QueryReferent(mDocWeak);
  if (!doc) return NS_ERROR_NOT_INITIALIZED;
  NS_ADDREF(*aDoc = doc);
  return NS_OK;
}


nsresult
nsEditor::GetPresShell(nsIPresShell **aPS)
{
  if (!aPS)
    return NS_ERROR_NULL_POINTER;
  *aPS = nsnull; // init out param
  NS_PRECONDITION(mPresShellWeak, "bad state, null mPresShellWeak");
  nsCOMPtr<nsIPresShell> ps = do_QueryReferent(mPresShellWeak);
  if (!ps) return NS_ERROR_NOT_INITIALIZED;
  NS_ADDREF(*aPS = ps);
  return NS_OK;
}


/* attribute string contentsMIMEType; */
NS_IMETHODIMP
nsEditor::GetContentsMIMEType(char * *aContentsMIMEType)
{
  NS_ENSURE_ARG_POINTER(aContentsMIMEType);
  *aContentsMIMEType = ToNewCString(mContentMIMEType);
  return NS_OK;
}

NS_IMETHODIMP
nsEditor::SetContentsMIMEType(const char * aContentsMIMEType)
{
  mContentMIMEType.Assign(aContentsMIMEType ? aContentsMIMEType : "");
  return NS_OK;
}

NS_IMETHODIMP
nsEditor::GetSelectionController(nsISelectionController **aSel)
{
  if (!aSel)
    return NS_ERROR_NULL_POINTER;
  *aSel = nsnull; // init out param
  NS_PRECONDITION(mSelConWeak, "bad state, null mSelConWeak");
  nsCOMPtr<nsISelectionController> selCon = do_QueryReferent(mSelConWeak);
  if (!selCon) return NS_ERROR_NOT_INITIALIZED;
  NS_ADDREF(*aSel = selCon);
  return NS_OK;
}


NS_IMETHODIMP
nsEditor::DeleteSelection(EDirection aAction)
{
  return DeleteSelectionImpl(aAction);
}

NS_IMETHODIMP
nsEditor::GetSelection(nsISelection **aSelection)
{
  if (!aSelection)
    return NS_ERROR_NULL_POINTER;
  *aSelection = nsnull;
  nsCOMPtr<nsISelectionController> selcon = do_QueryReferent(mSelConWeak);
  if (!selcon) return NS_ERROR_NOT_INITIALIZED;
  return selcon->GetSelection(nsISelectionController::SELECTION_NORMAL, aSelection);  // does an addref
}

NS_IMETHODIMP
nsEditor::DoTransaction(nsITransaction *aTxn)
{
#ifdef NS_DEBUG_EDITOR
  if (gNoisy) { printf("Editor::DoTransaction ----------\n"); }
#endif

  nsresult result = NS_OK;

  if (mPlaceHolderBatch && !mPlaceHolderTxn)
  {
    // it's pretty darn amazing how many different types of pointers
    // this transaction goes through here.  I bet this is a record.

    // We start off with an EditTxn since that's what the factory returns.
    nsRefPtr<EditTxn> editTxn;
    result = TransactionFactory::GetNewTransaction(PlaceholderTxn::GetCID(),
                                                   getter_AddRefs(editTxn));
    if (NS_FAILED(result)) { return result; }
    if (!editTxn) { return NS_ERROR_NULL_POINTER; }

    // Then we QI to an nsIAbsorbingTransaction to get at placeholder functionality
    nsCOMPtr<nsIAbsorbingTransaction> plcTxn;
    editTxn->QueryInterface(NS_GET_IID(nsIAbsorbingTransaction), getter_AddRefs(plcTxn));
    // have to use line above instead of "plcTxn = do_QueryInterface(editTxn);"
    // due to our broken interface model for transactions.

    // save off weak reference to placeholder txn
    mPlaceHolderTxn = do_GetWeakReference(plcTxn);
    plcTxn->Init(mPlaceHolderName, mSelState, this);
    mSelState = nsnull;  // placeholder txn took ownership of this pointer

    // finally we QI to an nsITransaction since that's what DoTransaction() expects
    nsCOMPtr<nsITransaction> theTxn = do_QueryInterface(plcTxn);
    DoTransaction(theTxn);  // we will recurse, but will not hit this case in the nested call

    if (mTxnMgr)
    {
      nsCOMPtr<nsITransaction> topTxn;
      result = mTxnMgr->PeekUndoStack(getter_AddRefs(topTxn));
      if (NS_FAILED(result)) return result;
      if (topTxn)
      {
        plcTxn = do_QueryInterface(topTxn);
        if (plcTxn)
        {
          // there is a placeholder transaction on top of the undo stack.  It is
          // either the one we just created, or an earlier one that we are now merging
          // into.  From here on out remember this placeholder instead of the one
          // we just created.
          mPlaceHolderTxn = do_GetWeakReference(plcTxn);
        }
      }
    }
  }

  if (aTxn)
  {
    // XXX: Why are we doing selection specific batching stuff here?
    // XXX: Most entry points into the editor have auto variables that
    // XXX: should trigger Begin/EndUpdateViewBatch() calls that will make
    // XXX: these selection batch calls no-ops.
    // XXX:
    // XXX: I suspect that this was placed here to avoid multiple
    // XXX: selection changed notifications from happening until after
    // XXX: the transaction was done. I suppose that can still happen
    // XXX: if an embedding application called DoTransaction() directly
    // XXX: to pump its own transactions through the system, but in that
    // XXX: case, wouldn't we want to use Begin/EndUpdateViewBatch() or
    // XXX: its auto equivalent nsAutoUpdateViewBatch to ensure that
    // XXX: selection listeners have access to accurate frame data?
    // XXX:
    // XXX: Note that if we did add Begin/EndUpdateViewBatch() calls
    // XXX: we will need to make sure that they are disabled during
    // XXX: the init of the editor for text widgets to avoid layout
    // XXX: re-entry during initial reflow. - kin

    // get the selection and start a batch change
    nsCOMPtr<nsISelection>selection;
    result = GetSelection(getter_AddRefs(selection));
    if (NS_FAILED(result)) { return result; }
    if (!selection) { return NS_ERROR_NULL_POINTER; }
    nsCOMPtr<nsISelectionPrivate>selPrivate(do_QueryInterface(selection));

    selPrivate->StartBatchChanges();

    if (mTxnMgr) {
      result = mTxnMgr->DoTransaction(aTxn);
    }
    else {
      result = aTxn->DoTransaction();
    }
    if (NS_SUCCEEDED(result)) {
      result = DoAfterDoTransaction(aTxn);
    }

    selPrivate->EndBatchChanges(); // no need to check result here, don't lose result of operation
  }

  NS_POSTCONDITION((NS_SUCCEEDED(result)), "transaction did not execute properly");

  return result;
}


NS_IMETHODIMP
nsEditor::EnableUndo(PRBool aEnable)
{
  nsresult result=NS_OK;

  if (PR_TRUE==aEnable)
  {
    if (!mTxnMgr)
    {
      mTxnMgr = do_CreateInstance(NS_TRANSACTIONMANAGER_CONTRACTID, &result);
      if (NS_FAILED(result) || !mTxnMgr) {
        return NS_ERROR_NOT_AVAILABLE;
      }
    }
    mTxnMgr->SetMaxTransactionCount(-1);
  }
  else
  { // disable the transaction manager if it is enabled
    if (mTxnMgr)
    {
      mTxnMgr->Clear();
      mTxnMgr->SetMaxTransactionCount(0);
    }
  }

  return NS_OK;
}


NS_IMETHODIMP
nsEditor::GetTransactionManager(nsITransactionManager* *aTxnManager)
{
  NS_ENSURE_ARG_POINTER(aTxnManager);

  *aTxnManager = NULL;
  if (!mTxnMgr)
    return NS_ERROR_FAILURE;

  NS_ADDREF(*aTxnManager = mTxnMgr);
  return NS_OK;
}

NS_IMETHODIMP
nsEditor::SetTransactionManager(nsITransactionManager *aTxnManager)
{
  NS_ENSURE_TRUE(aTxnManager, NS_ERROR_FAILURE);

  mTxnMgr = aTxnManager;
  return NS_OK;
}

NS_IMETHODIMP
nsEditor::Undo(PRUint32 aCount)
{
#ifdef NS_DEBUG_EDITOR
  if (gNoisy) { printf("Editor::Undo ----------\n"); }
#endif

  nsresult result = NS_OK;
  ForceCompositionEnd();

  PRBool hasTxnMgr, hasTransaction = PR_FALSE;
  CanUndo(&hasTxnMgr, &hasTransaction);
  if (!hasTransaction)
    return result;

  nsAutoRules beginRulesSniffing(this, kOpUndo, nsIEditor::eNone);

  if ((nsITransactionManager *)nsnull!=mTxnMgr.get())
  {
    PRUint32 i=0;
    for ( ; i<aCount; i++)
    {
      result = mTxnMgr->UndoTransaction();

      if (NS_SUCCEEDED(result))
        result = DoAfterUndoTransaction();

      if (NS_FAILED(result))
        break;
    }
  }

  NotifyEditorObservers();
//  msiUtils::Refresh(this);
  return result;
}


NS_IMETHODIMP nsEditor::CanUndo(PRBool *aIsEnabled, PRBool *aCanUndo)
{
  if (!aIsEnabled || !aCanUndo)
     return NS_ERROR_NULL_POINTER;
  *aIsEnabled = ((PRBool)((nsITransactionManager *)0!=mTxnMgr.get()));
  if (*aIsEnabled)
  {
    PRInt32 numTxns=0;
    mTxnMgr->GetNumberOfUndoItems(&numTxns);
    *aCanUndo = ((PRBool)(0!=numTxns));
  }
  else {
    *aCanUndo = PR_FALSE;
  }
  return NS_OK;
}


NS_IMETHODIMP
nsEditor::Redo(PRUint32 aCount)
{
#ifdef NS_DEBUG_EDITOR
  if (gNoisy) { printf("Editor::Redo ----------\n"); }
#endif

  nsresult result = NS_OK;

  PRBool hasTxnMgr, hasTransaction = PR_FALSE;
  CanRedo(&hasTxnMgr, &hasTransaction);
  if (!hasTransaction)
    return result;

  nsAutoRules beginRulesSniffing(this, kOpRedo, nsIEditor::eNone);

  if ((nsITransactionManager *)nsnull!=mTxnMgr.get())
  {
    PRUint32 i=0;
    for ( ; i<aCount; i++)
    {
      result = mTxnMgr->RedoTransaction();

      if (NS_SUCCEEDED(result))
        result = DoAfterRedoTransaction();

      if (NS_FAILED(result))
        break;
    }
  }

  NotifyEditorObservers();
  return result;
}


NS_IMETHODIMP nsEditor::CanRedo(PRBool *aIsEnabled, PRBool *aCanRedo)
{
  if (!aIsEnabled || !aCanRedo)
     return NS_ERROR_NULL_POINTER;

  *aIsEnabled = ((PRBool)((nsITransactionManager *)0!=mTxnMgr.get()));
  if (*aIsEnabled)
  {
    PRInt32 numTxns=0;
    mTxnMgr->GetNumberOfRedoItems(&numTxns);
    *aCanRedo = ((PRBool)(0!=numTxns));
  }
  else {
    *aCanRedo = PR_FALSE;
  }
  return NS_OK;
}


NS_IMETHODIMP
nsEditor::BeginTransaction()
{
  BeginUpdateViewBatch();

  if ((nsITransactionManager *)nsnull!=mTxnMgr.get())
  {
    mTxnMgr->BeginBatch();
  }

  return NS_OK;
}

NS_IMETHODIMP
nsEditor::EndTransaction()
{
  if ((nsITransactionManager *)nsnull!=mTxnMgr.get())
  {
    mTxnMgr->EndBatch();
  }

  EndUpdateViewBatch();

  return NS_OK;
}


// These two routines are similar to the above, but do not use
// the transaction managers batching feature.  Instead we use
// a placeholder transaction to wrap up any further transaction
// while the batch is open.  The advantage of this is that
// placeholder transactions can later merge, if needed.  Merging
// is unavailable between transaction manager batches.

NS_IMETHODIMP
nsEditor::BeginPlaceHolderTransaction(nsIAtom *aName)
{
  NS_PRECONDITION(mPlaceHolderBatch >= 0, "negative placeholder batch count!");
  if (!mPlaceHolderBatch)
  {
    // time to turn on the batch
    BeginUpdateViewBatch();
    mPlaceHolderTxn = nsnull;
    mPlaceHolderName = aName;
    nsCOMPtr<nsISelection> selection;
    nsresult res = GetSelection(getter_AddRefs(selection));
    if (NS_FAILED(res)) return res;
    mSelState = new nsSelectionState();
    if (!mSelState)
      return NS_ERROR_OUT_OF_MEMORY;

    mSelState->SaveSelection(selection);
  }
  mPlaceHolderBatch++;

  return NS_OK;
}

NS_IMETHODIMP
nsEditor::EndPlaceHolderTransaction()
{
  NS_PRECONDITION(mPlaceHolderBatch > 0, "zero or negative placeholder batch count when ending batch!");
  if (mPlaceHolderBatch == 1)
  {
    nsCOMPtr<nsISelection>selection;
    nsresult rv = GetSelection(getter_AddRefs(selection));

    nsCOMPtr<nsISelectionPrivate>selPrivate(do_QueryInterface(selection));

   // By making the assumption that no reflow happens during the calls
   // to EndUpdateViewBatch and ScrollSelectionIntoView, we are able to
   // allow the selection to cache a frame offset which is used by the
   // caret drawing code. We only enable this cache here; at other times,
   // we have no way to know whether reflow invalidates it
   // See bugs 35296 and 199412.
    if (selPrivate) {
      selPrivate->SetCanCacheFrameOffset(PR_TRUE);
    }

    // time to turn off the batch
    EndUpdateViewBatch();
    // make sure selection is in view

    // After ScrollSelectionIntoView(), the pending notifications might be
    // flushed and PresShell/PresContext/Frames may be dead. See bug 418470.
    ScrollSelectionIntoView(PR_FALSE);

    // cached for frame offset are Not available now
    if (selPrivate) {
      selPrivate->SetCanCacheFrameOffset(PR_FALSE);
    }

    if (mSelState)
    {
      // we saved the selection state, but never got to hand it to placeholder
      // (else we ould have nulled out this pointer), so destroy it to prevent leaks.
      delete mSelState;
      mSelState = nsnull;
    }
    if (mPlaceHolderTxn)  // we might have never made a placeholder if no action took place
    {
      nsCOMPtr<nsIAbsorbingTransaction> plcTxn = do_QueryReferent(mPlaceHolderTxn);
      if (plcTxn)
      {
        plcTxn->EndPlaceHolderBatch();
      }
      else
      {
        // in the future we will check to make sure undo is off here,
        // since that is the only known case where the placeholdertxn would disappear on us.
        // For now just removing the assert.
      }
      // notify editor observers of action unless it is uncommitted IME
      if (!mInIMEMode) NotifyEditorObservers();
    }
  }
  mPlaceHolderBatch--;

  return NS_OK;
}

NS_IMETHODIMP
nsEditor::ShouldTxnSetSelection(PRBool *aResult)
{
  if (!aResult) return NS_ERROR_NULL_POINTER;
  *aResult = mShouldTxnSetSelection;
  return NS_OK;
}

NS_IMETHODIMP
nsEditor::SetShouldTxnSetSelection(PRBool aShould)
{
  mShouldTxnSetSelection = aShould;
  return NS_OK;
}

NS_IMETHODIMP
nsEditor::GetDocumentIsEmpty(PRBool *aDocumentIsEmpty)
{
  *aDocumentIsEmpty = PR_TRUE;

  nsIDOMElement *rootElement = GetRoot();
  if (!rootElement)
    return NS_ERROR_NULL_POINTER;

  PRBool hasChildNodes;
  nsresult res = rootElement->HasChildNodes(&hasChildNodes);

  *aDocumentIsEmpty = !hasChildNodes;

  return res;
}


// XXX: the rule system should tell us which node to select all on (ie, the root, or the body)
NS_IMETHODIMP nsEditor::SelectAll()
{
  if (!mDocWeak || !mPresShellWeak) { return NS_ERROR_NOT_INITIALIZED; }
  ForceCompositionEnd();

  nsCOMPtr<nsISelectionController> selCon = do_QueryReferent(mSelConWeak);
  if (!selCon) return NS_ERROR_NOT_INITIALIZED;
  nsCOMPtr<nsISelection> selection;
  nsresult result = selCon->GetSelection(nsISelectionController::SELECTION_NORMAL, getter_AddRefs(selection));
  if (NS_SUCCEEDED(result) && selection)
  {
    result = SelectEntireDocument(selection);
  }
  return result;
}

NS_IMETHODIMP nsEditor::BeginningOfDocument()
{
  if (!mDocWeak || !mPresShellWeak) { return NS_ERROR_NOT_INITIALIZED; }

  // get the selection
  nsCOMPtr<nsISelection> selection;
  nsresult result = GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(result))
    return result;
  if (!selection)
    return NS_ERROR_NOT_INITIALIZED;

  // get the root element
  nsIDOMElement *rootElement = GetRoot();
  if (!rootElement)
    return NS_ERROR_NULL_POINTER;

  // find first editable thingy
  nsCOMPtr<nsIDOMNode> firstNode;
  result = GetFirstEditableNode(rootElement, address_of(firstNode));
  if (firstNode)
  {
    // if firstNode is text, set selection to beginning of the text node
    if (IsTextNode(firstNode))
    {
      result = selection->Collapse(firstNode, 0);
    }
    else
    { // otherwise, it's a leaf node and we set the selection just in front of it
      nsCOMPtr<nsIDOMNode> parentNode;
      result = firstNode->GetParentNode(getter_AddRefs(parentNode));
      if (NS_FAILED(result)) { return result; }
      if (!parentNode) { return NS_ERROR_NULL_POINTER; }
      PRInt32 offsetInParent;
      result = nsEditor::GetChildOffset(firstNode, parentNode, offsetInParent);
      if (NS_FAILED(result)) return result;
      result = selection->Collapse(parentNode, offsetInParent);
    }
  }
  else
  {
    // just the root node, set selection to inside the root
    result = selection->Collapse(rootElement, 0);
  }
  nsEditorUtils::JiggleCursor(this, selection, eNext);

  return result;
}

NS_IMETHODIMP
nsEditor::EndOfDocument()
{
  if (!mDocWeak || !mPresShellWeak) { return NS_ERROR_NOT_INITIALIZED; }
  nsresult res;

  // get selection
  nsCOMPtr<nsISelection> selection;
  res = GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(res)) return res;
  if (!selection)   return NS_ERROR_NULL_POINTER;

  // get the root element
  nsIDOMElement *rootElement = GetRoot();
  if (!rootElement)
    return NS_ERROR_NULL_POINTER;

  // get the length of the root element
  PRUint32 len;
  res = GetLengthOfDOMNode(rootElement, len);
  if (NS_FAILED(res)) return res;

  // set the selection to after the last child of the root element
  res = selection->Collapse(rootElement, (PRInt32)len);
  nsEditorUtils::JiggleCursor(this, selection, ePrevious);
  return res;
}

NS_IMETHODIMP
nsEditor::GetDocumentModified(PRBool *outDocModified)
{
  if (!outDocModified)
    return NS_ERROR_NULL_POINTER;

  PRInt32  modCount = 0;
  GetModificationCount(&modCount);

  *outDocModified = (modCount != 0);
  return NS_OK;
}

NS_IMETHODIMP
nsEditor::GetDocumentCharacterSet(nsACString &characterSet)
{
  nsCOMPtr<nsIPresShell> presShell;

  nsresult rv = GetPresShell(getter_AddRefs(presShell));
  if (NS_SUCCEEDED(rv))
  {
    nsIDocument *doc = presShell->GetDocument();
    if (doc) {
      characterSet = doc->GetDocumentCharacterSet();
      return NS_OK;
    }
    rv = NS_ERROR_NULL_POINTER;
  }

  return rv;

}

NS_IMETHODIMP
nsEditor::SetDocumentCharacterSet(const nsACString& characterSet)
{
  nsCOMPtr<nsIPresShell> presShell;
  nsresult rv = GetPresShell(getter_AddRefs(presShell));
  if (NS_SUCCEEDED(rv))
  {
    nsIDocument *doc = presShell->GetDocument();
    if (doc) {
      doc->SetDocumentCharacterSet(characterSet);
      return NS_OK;
    }
    rv = NS_ERROR_NULL_POINTER;
  }

  return rv;
}

//
// Get an appropriate wrap width for saving this document.
// This class just uses a pref; subclasses are expected to
// override if they know more about the document.
//
NS_IMETHODIMP
nsEditor::GetWrapWidth(PRInt32 *aWrapColumn)
{
  NS_ENSURE_ARG_POINTER(aWrapColumn);
  *aWrapColumn = 72;

  nsresult rv;
  nsCOMPtr<nsIPrefBranch> prefBranch =
    do_GetService(NS_PREFSERVICE_CONTRACTID, &rv);
  if (NS_SUCCEEDED(rv) && prefBranch)
    (void) prefBranch->GetIntPref("editor.htmlWrapColumn", aWrapColumn);
  return NS_OK;
}

NS_IMETHODIMP
nsEditor::Cut()
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsEditor::CanCut(PRBool *aCanCut)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsEditor::Copy()
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsEditor::CanCopy(PRBool *aCanCut)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsEditor::Paste(PRInt32 aSelectionType)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsEditor::CanPaste(PRInt32 aSelectionType, PRBool *aCanPaste)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsEditor::CanDrag(nsIDOMEvent *aEvent, PRBool *aCanDrag)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsEditor::DoDrag(nsIDOMEvent *aEvent)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsEditor::InsertFromDrop(nsIDOMEvent *aEvent)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}


NS_IMETHODIMP
nsEditor::SetAttribute(nsIDOMElement *aElement, const nsAString & aAttribute, const nsAString & aValue)
{
  nsRefPtr<ChangeAttributeTxn> txn;
  nsresult result = CreateTxnForSetAttribute(aElement, aAttribute, aValue,
                                             getter_AddRefs(txn));
  if (NS_SUCCEEDED(result))  {
    result = DoTransaction(txn);
  }
  return result;
}

NS_IMETHODIMP
nsEditor::GetAttributeValue(nsIDOMElement *aElement,
                            const nsAString & aAttribute,
                            nsAString & aResultValue,
                            PRBool *aResultIsSet)
{
  if (!aResultIsSet)
    return NS_ERROR_NULL_POINTER;
  *aResultIsSet=PR_FALSE;
  nsresult result=NS_OK;
  if (aElement)
  {
    nsCOMPtr<nsIDOMAttr> attNode;
    result = aElement->GetAttributeNode(aAttribute, getter_AddRefs(attNode));
    if ((NS_SUCCEEDED(result)) && attNode)
    {
      attNode->GetSpecified(aResultIsSet);
      attNode->GetValue(aResultValue);
    }
  }
  return result;
}

NS_IMETHODIMP
nsEditor::RemoveAttribute(nsIDOMElement *aElement, const nsAString& aAttribute)
{
  nsRefPtr<ChangeAttributeTxn> txn;
  nsresult result = CreateTxnForRemoveAttribute(aElement, aAttribute,
                                                getter_AddRefs(txn));
  if (NS_SUCCEEDED(result))  {
    result = DoTransaction(txn);
  }
  return result;
}


NS_IMETHODIMP
nsEditor::MarkNodeDirty(nsIDOMNode* aNode)
{
  //  mark the node dirty.
  nsCOMPtr<nsIDOMElement> element (do_QueryInterface(aNode));
  if (element)
    element->SetAttribute(NS_LITERAL_STRING("_moz_dirty"), EmptyString());
  return NS_OK;
}

NS_IMETHODIMP nsEditor::GetInlineSpellChecker(PRBool autoCreate,
                                              nsIInlineSpellChecker ** aInlineSpellChecker)
{
  NS_ENSURE_ARG_POINTER(aInlineSpellChecker);

  if (mDidPreDestroy) {
    // Don't allow people to get or create the spell checker once the editor
    // is going away.
    *aInlineSpellChecker = nsnull;
    return autoCreate ? NS_ERROR_NOT_AVAILABLE : NS_OK;
  }

  if (!mInlineSpellChecker && autoCreate) {
    nsresult rv;
    mInlineSpellChecker = do_CreateInstance(MOZ_INLINESPELLCHECKER_CONTRACTID, &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = mInlineSpellChecker->Init(this);
    if (NS_FAILED(rv))
      mInlineSpellChecker = nsnull;
    NS_ENSURE_SUCCESS(rv, rv);
  }

  NS_IF_ADDREF(*aInlineSpellChecker = mInlineSpellChecker);

  return NS_OK;
}

NS_IMETHODIMP nsEditor::SyncRealTimeSpell()
{
  PRBool enable = GetDesiredSpellCheckState();

  nsCOMPtr<nsIInlineSpellChecker> spellChecker;
  GetInlineSpellChecker(enable, getter_AddRefs(spellChecker));

  if (spellChecker) {
    spellChecker->SetEnableRealTimeSpell(enable);
  }

  return NS_OK;
}

NS_IMETHODIMP nsEditor::SetSpellcheckUserOverride(PRBool enable)
{
  mSpellcheckCheckboxState = enable ? eTriTrue : eTriFalse;

  return SyncRealTimeSpell();
}


NS_IMETHODIMP nsEditor::CreateNode(const nsAString& aTag,
                                   nsIDOMNode *    aParent,
                                   PRInt32         aPosition,
                                   nsIDOMNode **   aNewNode)
{
  PRInt32 i;

  nsAutoRules beginRulesSniffing(this, kOpCreateNode, nsIEditor::eNext);

  for (i = 0; i < mActionListeners.Count(); i++)
    mActionListeners[i]->WillCreateNode(aTag, aParent, aPosition);

  nsRefPtr<CreateElementTxn> txn;
  nsresult result = CreateTxnForCreateElement(aTag, aParent, aPosition,
                                              getter_AddRefs(txn));
  if (NS_SUCCEEDED(result))
  {
    result = DoTransaction(txn);
    if (NS_SUCCEEDED(result))
    {
      result = txn->GetNewNode(aNewNode);
      NS_IF_ADDREF(*aNewNode);
      NS_ASSERTION((NS_SUCCEEDED(result)), "GetNewNode can't fail if txn::DoTransaction succeeded.");
    }
  }

  mRangeUpdater.SelAdjCreateNode(aParent, aPosition);

  for (i = 0; i < mActionListeners.Count(); i++)
    mActionListeners[i]->DidCreateNode(aTag, *aNewNode, aParent, aPosition, result);

  return result;
}

/**
 * Sometimes we need to put a node where the rules do not allow it. A common example is inserting text into the
 * body tag or into a td tag. We do not allow this, but before aborting, we check to see if we can put in a default
 * paragraph tag to go between the text tag and the body or table tag.
 *
 * node -- the node being inserted
 * parent -- in: the node into which we are inserting
 *           out: the final node we are inserting into; due to node splitting, it may be an ancestor of the original parent.
 * aPosition -- the offset in parent where we are inserting
 * _retval -- the final offset into which we are inserting. It may be different from aPosition because of node splitting.
 */

NS_IMETHODIMP nsEditor::InsertBufferNodeIfNeeded(nsIDOMNode*    node,
                                                 nsIDOMNode **  outNode,
                                                 nsIDOMNode *   parent,
                                                 nsIDOMNode **  outParent,
                                                 PRInt32        aPosition,
                                                 PRInt32 *      _retval)
{
  nsresult res = NS_OK;
  nsAutoString nodeName;   // The tag of the node we are inserting (node)
  node->GetNodeName(nodeName);
  nsAutoString parentName;  // The tag of the current candidate for parent
  nsAutoString tagclass;
  nsAutoString frametype;
  nsCOMPtr<nsIDOMNode> topChild = do_QueryInterface(parent);
  nsCOMPtr<nsIDOMNode> tmp;
  nsCOMPtr<nsIDOMNode> ptr = parent;   // the current candidate for parent
  ptr->GetNodeName(parentName);
  nsCOMPtr<nsIDOMElement> parentElt;
  *outNode = node;
  NS_ADDREF(node);  // BBM: justification for this??
  *outParent = parent;
  NS_ADDREF(parent);  // BBM: justification for this??
  // We want to keep from doing this for frames of graphs and graphics, so we check for that here. This should someday be moved to the node defs mechanism
  if (nodeName.EqualsLiteral("msiframe")) {
    parentElt = do_QueryInterface(parent);
    if (parentElt) {
      parentElt->GetAttribute(NS_LITERAL_STRING("frametype"), frametype);
      if (frametype.EqualsLiteral("image") || frametype.EqualsLiteral("msigraph")) {
        *_retval = -1;
        return NS_ERROR_FAILURE;
      }
    }
  }
  PRInt32 offsetOfInsert = aPosition;
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface((nsIEditor*)this);
  if (!htmlEditor){
    *_retval = -1;
    return NS_ERROR_FAILURE;
  }
  nsCOMPtr<msiITagListManager> tlm;
  htmlEditor->GetTagListManager(getter_AddRefs(tlm));
  nsCOMPtr<nsIContent> content = do_QueryInterface(node);
  if (content->TextIsOnlyWhitespace())
    tlm->GetRealClassOfTag(nodeName, nsnull, tagclass);
    // Search up the parent chain to find a suitable container
  while (!CanContainTag(ptr, nodeName)) // && !(content && content->TextIsOnlyWhitespace()))
  {
    // If the current parent is a root (body or table element)
    // then go no further - we can't insert. See if interposing a default paragraph helps.
    if (nsTextEditUtils::IsBody(ptr) || nsHTMLEditUtils::IsTableElement(ptr, tlm)) // || nsHTMLEditUtils::IsListItem(ptr, tlm) ||
        // nsHTMLEditUtils::IsStructNode(ptr, tlm) || nsHTMLEditUtils::IsEnvNode(ptr, tlm))
    {
      nsCOMPtr<nsIDOMNode> para;
      htmlEditor->CreateDefaultParagraph(parent, aPosition, PR_FALSE, getter_AddRefs(para));
      if (!para || !CanContainTag(para, nodeName))
      {
        return NS_ERROR_FAILURE;
      }
      *outParent = para;
      offsetOfInsert = 0;
      break;
    }
    else
    {
      // Get the next parent
      if (parentName.EqualsLiteral("graph") || parentName.EqualsLiteral("img") || parentName.EqualsLiteral("object")) {
        *_retval = -1;
        return NS_ERROR_FAILURE;  // *_retval = -1 means there is no place for the insertion.
      }
      ptr->GetParentNode(getter_AddRefs(tmp));
      NS_ENSURE_TRUE(tmp, NS_ERROR_FAILURE);
      topChild = ptr;
      ptr = tmp;
      ptr->GetNodeName(parentName);
    }
  }
  if (ptr != topChild) //If suspicious, check the history of this line
  {
    // we need to split some levels above the original selection parent
    res = SplitNodeDeep(topChild, parent, offsetOfInsert, &offsetOfInsert, PR_TRUE);
    *outParent = ptr;
    if (NS_FAILED(res)) {
      *_retval= -1;
      return NS_ERROR_FAILURE;
    }
  }
  *_retval = offsetOfInsert;
}


NS_IMETHODIMP nsEditor::InsertNode(nsIDOMNode * aNode,
                                   nsIDOMNode * aParent,
                                   PRInt32      aPosition)
{
  PRInt32 i;
  PRInt32 offset = aPosition;
  nsresult result;
  nsAutoString nodeName;
  PRBool fNeedToValidate = PR_FALSE;
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface((nsIEditor*)this);
//  nsCOMPtr<nsIDOMElement> element(do_QueryInterface(aNode));
  // check for well-formed mathematics in case this is coming from the clipboard
  result = aNode->GetNodeName(nodeName);
  if (nsHTMLEditUtils::IsMath(aNode) || nodeName.EqualsLiteral("#document-fragment")) {  
    // if (htmlEditor) {
      // fNeedToValidate = PR_TRUE;
    // }
  }
  nsAutoRules beginRulesSniffing(this, kOpInsertNode, nsIEditor::eNext);
  for (i = 0; i < mActionListeners.Count(); i++)
    mActionListeners[i]->WillInsertNode(aNode, aParent, aPosition);

  nsRefPtr<InsertElementTxn> txn;
  result = CreateTxnForInsertElement(aNode, aParent, aPosition,
                                              getter_AddRefs(txn));
  if (NS_SUCCEEDED(result))  {
    result = DoTransaction(txn);
  }
  if (fNeedToValidate) {
    htmlEditor->ValidateMathSyntax(aNode, PR_FALSE, PR_FALSE);
  }
  mRangeUpdater.SelAdjInsertNode(aParent, aPosition);

  for (i = 0; i < mActionListeners.Count(); i++)
    mActionListeners[i]->DidInsertNode(aNode, aParent, aPosition, result);
  return result;
}

NS_IMETHODIMP nsEditor::ReplaceNode(nsIDOMNode * aNewChild,
                                    nsIDOMNode * aOldChild,
                                    nsIDOMNode * aParent )
{
  PRInt32 i;
  nsIEditActionListener *listener;
  nsAutoRules beginRulesSniffing(this, kOpReplaceNode, nsIEditor::eNone); //ljh i think eNone is correct???

  for (i = 0; i < mActionListeners.Count(); i++)
  {
    listener = (nsIEditActionListener *)mActionListeners[i];
    if (listener)
    {
      nsCOMPtr<msiIEditActionListenerExtension> msiListener(do_QueryInterface(listener));
      if (msiListener)
        msiListener->WillReplaceNode(aNewChild, aOldChild, aParent);
    }
  }

  //ReplaceElementTxn *txn;
  nsRefPtr<ReplaceElementTxn> txn;
  nsresult result = CreateTxnForReplaceElement(aNewChild, aOldChild, aParent, PR_TRUE, getter_AddRefs(txn));
  if (NS_SUCCEEDED(result))
  {
    result = DoTransaction(txn);
  }
  //// The transaction system (if any) has taken ownwership of txn
  //NS_IF_RELEASE(txn);

  for (i = 0; i < mActionListeners.Count(); i++)
  {
    listener = (nsIEditActionListener *)mActionListeners[i];
    if (listener)
    {
      nsCOMPtr<msiIEditActionListenerExtension> msiListener(do_QueryInterface(listener));
      if (msiListener)
      msiListener->DidReplaceNode(aNewChild, aOldChild, aParent, result);
    }
  }

  return result;
}

NS_IMETHODIMP nsEditor::SaveSelection(nsISelection * selection)
{

//ljh TODO -- do we need this listener stuff???
//  PRInt32 i;
//  nsIEditActionListener *listener;
//  nsAutoRules beginRulesSniffing(this, kOpReplaceNode, nsIEditor::eNone); //ljh i think eNone is correct???
//
//  if (mActionListeners)
//  {
//    for (i = 0; i < mActionListeners->Count(); i++)
//    {
//      listener = (nsIEditActionListener *)mActionListeners->ElementAt(i);
//      if (listener)
//        listener->WillReplaceNode(aNewChild, aOldChild, aParent);
//    }
//  }

  nsRefPtr<PlaceholderTxn> txn;
  nsresult result = CreateTxnForSaveSelection(selection, getter_AddRefs(txn));
  if (NS_SUCCEEDED(result))
  {
    result = DoTransaction(txn);
  }

  return result;
}


NS_IMETHODIMP
nsEditor::SplitNode(nsIDOMNode * aNode,
                    PRInt32      aOffset,
                    nsIDOMNode **aNewLeftNode)
{
  PRInt32 i;

  nsAutoRules beginRulesSniffing(this, kOpSplitNode, nsIEditor::eNext);

  for (i = 0; i < mActionListeners.Count(); i++)
    mActionListeners[i]->WillSplitNode(aNode, aOffset);

  nsRefPtr<SplitElementTxn> txn;
  nsresult result = CreateTxnForSplitNode(aNode, aOffset, getter_AddRefs(txn));
  if (NS_SUCCEEDED(result))
  {
    result = DoTransaction(txn);
    if (NS_SUCCEEDED(result))
    {
      result = txn->GetNewNode(aNewLeftNode);
      NS_ASSERTION((NS_SUCCEEDED(result)), "result must succeeded for GetNewNode");
    }
  }

  mRangeUpdater.SelAdjSplitNode(aNode, aOffset, *aNewLeftNode);

  for (i = 0; i < mActionListeners.Count(); i++)
  {
    nsIDOMNode *ptr = *aNewLeftNode;
    mActionListeners[i]->DidSplitNode(aNode, aOffset, ptr, result);
  }

  return result;
}



NS_IMETHODIMP
nsEditor::JoinNodes(nsIDOMNode * aLeftNode,
                    nsIDOMNode * aRightNode,
                    nsIDOMNode * aParent)
{
  PRInt32 i, offset;
  nsAutoRules beginRulesSniffing(this, kOpJoinNode, nsIEditor::ePrevious);

  // remember some values; later used for saved selection updating.
  // find the offset between the nodes to be joined.
  nsresult result = GetChildOffset(aRightNode, aParent, offset);
  if (NS_FAILED(result)) return result;
  // find the number of children of the lefthand node
  PRUint32 oldLeftNodeLen;
  result = GetLengthOfDOMNode(aLeftNode, oldLeftNodeLen);
  if (NS_FAILED(result)) return result;

  for (i = 0; i < mActionListeners.Count(); i++)
    mActionListeners[i]->WillJoinNodes(aLeftNode, aRightNode, aParent);

  nsRefPtr<JoinElementTxn> txn;
  result = CreateTxnForJoinNode(aLeftNode, aRightNode, getter_AddRefs(txn));
  if (NS_SUCCEEDED(result))  {
    result = DoTransaction(txn);
  }

  mRangeUpdater.SelAdjJoinNodes(aLeftNode, aRightNode, aParent, offset, (PRInt32)oldLeftNodeLen);

  for (i = 0; i < mActionListeners.Count(); i++)
    mActionListeners[i]->DidJoinNodes(aLeftNode, aRightNode, aParent, result);

  return result;
}


NS_IMETHODIMP nsEditor::DeleteNode(nsIDOMNode * aElement)
{
  PRInt32 i, offset;
  nsCOMPtr<nsIDOMNode> parent;
  nsAutoRules beginRulesSniffing(this, kOpCreateNode, nsIEditor::ePrevious);
  nsCOMPtr<nsISelection>selection;

  // save node location for selection updating code.
  nsresult result = GetNodeLocation(aElement, address_of(parent), &offset);
  if (NS_FAILED(result)) return result;
  result = GetSelection(getter_AddRefs(selection));
  // nsCOMPtr<nsIDOMRange> range;
  // selection->GetRangeAt(0, getter_AddRefs(range));
  for (i = 0; i < mActionListeners.Count(); i++)
    mActionListeners[i]->WillDeleteNode(aElement);

  nsRefPtr<DeleteElementTxn> txn;
  result = CreateTxnForDeleteElement(aElement, getter_AddRefs(txn));
  if (NS_SUCCEEDED(result))  {
    nsAutoTrackDOMPoint tracker(mRangeUpdater, address_of(parent), &offset);
    result = DoTransaction(txn);
  }

  for (i = 0; i < mActionListeners.Count(); i++)
    mActionListeners[i]->DidDeleteNode(aElement, result);

  // BBM: commented out 2020.10.21 selection->Collapse(parent, offset);

  return result;
}

nsresult
nsEditor::SetSelectionOnCursorTag(nsIDOMNode * node, PRBool * setCursor)
{
  NS_ENSURE_TRUE(node, NS_ERROR_NULL_POINTER);
  nsCOMPtr<nsIDOMElement> element;
  nsCOMPtr<nsIDOMNodeList> nodeList;
  nsCOMPtr<nsIDOMNode> selNode;
  nsCOMPtr<nsIDOMNode> cursorNode;
  PRUint32 nodeCount = 0;
  PRInt32 selOffset;
  PRBool rv = PR_FALSE;
  nsCOMPtr<nsISelection> selection;
  element = do_QueryInterface(node);
  element->GetElementsByTagName(NS_LITERAL_STRING("cursor"), getter_AddRefs(nodeList));
  if (nodeList) nodeList->GetLength(&nodeCount);
  if (nodeCount > 0)
  {
    rv = PR_TRUE;
    nodeList->Item(0, getter_AddRefs(cursorNode));
    GetNodeLocation(cursorNode, address_of(selNode), &selOffset);
    DeleteNode(cursorNode);
//    selPriv->SetInterlinePosition(PR_TRUE);
    GetSelection(getter_AddRefs(selection));
    selection->Collapse(selNode, selOffset);
  }
  *setCursor = rv;
  return NS_OK;
}

///////////////////////////////////////////////////////////////////////////
// ReplaceContainer: replace inNode with a new node (outNode) which is constructed
//                   to be of type aNodeType.  Put inNode's children into outNode.
//                   It is the caller's responsibility to make sure inNode's children can
//                   go in outNode.
NS_IMETHODIMP
nsEditor::ReplaceContainer(nsIDOMNode *inNode,
                            const nsAString & newTag,
                            msiITagListManager *manager,
                            const nsAString & anAttribute,
                            const nsAString & aValue,
                            PRBool cloneAttributes,
                            nsIDOMNode **_outNode)
{
  // return NS_OK;

  if (!inNode || !_outNode)
    return NS_ERROR_NULL_POINTER;
  nsCOMPtr<nsIDOMNode> parent;
  nsCOMPtr<nsIDOMNode> newNode;
  nsCOMPtr<nsIDOMNode> child;
  nsCOMPtr<nsINode> childNode;
  nsCOMPtr<nsIDOMElement> elem;
  PRInt32 offset;
  PRBool setCursor = PR_FALSE;
  nsresult res = GetNodeLocation(inNode, address_of(parent), &offset);
  if (NS_FAILED(res)) return res;

  // create new container
  nsCOMPtr<nsIDOMDocument> doc;
  res = parent->GetOwnerDocument(getter_AddRefs(doc));
  res = manager->GetNewInstanceOfNode(newTag, nsnull, doc, getter_AddRefs(newNode));
  if (newNode)
  {
//    *_outNode = newNode;
    res = doc->ImportNode((nsIDOMNode*)newNode, PR_TRUE, (nsIDOMNode**)(_outNode));
    elem = do_QueryInterface(*_outNode);
    nsCOMPtr<nsINode> elemNode(do_QueryInterface(*_outNode));
    elemNode->SetEditableFlag(PR_TRUE);
    res = InsertNode(*_outNode, parent, offset);
    nsCOMPtr<nsIAtom> nsAtom = nsnull;
    // PRBool isFrontMatterTag;
    // manager->GetTagInClass(NS_LITERAL_STRING("frontmtag"), newTag, nsAtom, &isFrontMatterTag);
       // since we are applying this to existing text, take out the contents of newNode
    // if (!isFrontMatterTag) {
      res = elem->GetFirstChild(getter_AddRefs(child));
      while (child && (res==NS_OK))
      {
        childNode = do_QueryInterface(child);
        childNode->SetEditableFlag(PR_TRUE);
        res = DeleteNode(child);
        if (res != NS_OK) break;
        res = elem->GetFirstChild(getter_AddRefs(child));
      }
    // }
  }
  else
  {
    res = CreateNode(newTag, parent, offset, (nsIDOMNode **)_outNode);
    elem = do_QueryInterface(*_outNode);
  }
  if (NS_FAILED(res)) return res;
  // reposition selection to inside the block
//	if (*_outNode) res = SetCursorInNewHTML(elem, (PRBool *) nsnull);


  // set attribute if needed
  if (!(anAttribute.IsEmpty()) && !(aValue.IsEmpty()))
  {
    res = elem->SetAttribute(anAttribute, aValue);
    if (NS_FAILED(res)) return res;
  }
  if (cloneAttributes)
  {
    nsCOMPtr<nsIDOMNode>newElementNode = do_QueryInterface(elem);
    res = CloneAttributes(newElementNode, inNode);
    if (NS_FAILED(res)) return res;
  }


  // notify our internal selection state listener
  // (Note: A nsAutoSelectionReset object must be created
  //  before calling this to initialize mRangeUpdater)
  // if (!setCursor) {
  nsAutoReplaceContainerSelNotify selStateNotify(mRangeUpdater, inNode, *_outNode);
  {
    nsAutoTxnsConserveSelection conserveSelection(this);
    PRBool bHasMoreChildren;
    inNode->HasChildNodes(&bHasMoreChildren);
    while (bHasMoreChildren)
    {
      inNode->GetFirstChild(getter_AddRefs(child));
      childNode = do_QueryInterface(child);
      childNode->SetEditableFlag(PR_TRUE);
      res = DeleteNode(child);
      if (NS_FAILED(res)) return res;

      if (!nsTextEditUtils::IsBreak(child))
      {
        res = InsertNode(child, *_outNode, -1);
        if (NS_FAILED(res)) return res;
      }
      inNode->HasChildNodes(&bHasMoreChildren);
    }
  }
  return DeleteNode(inNode);
}

NS_IMETHODIMP
nsEditor::ReplaceContainerNode( nsIDOMElement * elt, nsIDOMElement * newElt ) // This is like editor::ReplaceContainer, except that the new container is
// already constructed, since we have to set the namespace sometimes. The two functions might be combined in the future.
{
  if (!elt || !newElt)
    return NS_ERROR_NULL_POINTER;
  nsCOMPtr<nsIDOMNode> parent;
  nsCOMPtr<nsIDOMNode> child;
  nsCOMPtr<nsINode> childNode;
  nsCOMPtr<nsIDOMElement> elem;
  PRInt32 offset;
  PRBool setCursor = PR_FALSE;
  nsresult res = GetNodeLocation(elt, address_of(parent), &offset);
  if (NS_FAILED(res)) return res;

  // newElt->SetEditableFlag(PR_TRUE);
  res = InsertNode(newElt, parent, offset);
  // nsCOMPtr<nsIAtom> nsAtom = nsnull;
  if (NS_FAILED(res)) return res;

  // // set attribute if needed
  // if (!(anAttribute.IsEmpty()) && !(aValue.IsEmpty()))
  // {
  //   res = elem->SetAttribute(anAttribute, aValue);
  //   if (NS_FAILED(res)) return res;
  // }

  // notify our internal selection state listener
  // (Note: A nsAutoSelectionReset object must be created
  //  before calling this to initialize mRangeUpdater)
  // if (!setCursor) {
  nsAutoReplaceContainerSelNotify selStateNotify(mRangeUpdater, elt, newElt);
  {
    nsAutoTxnsConserveSelection conserveSelection(this);
    PRBool bHasMoreChildren;
    elt->HasChildNodes(&bHasMoreChildren);
    while (bHasMoreChildren)
    {
      elt->GetFirstChild(getter_AddRefs(child));
      childNode = do_QueryInterface(child);
      childNode->SetEditableFlag(PR_TRUE);
      res = DeleteNode(child);
//      if (NS_FAILED(res)) return res;

//      if (!nsTextEditUtils::IsBreak(child))
//      {
        res = InsertNode(child, newElt, -1);
        if (NS_FAILED(res)) return res;
//      }
      elt->HasChildNodes(&bHasMoreChildren);
    }
  }
  return DeleteNode(elt);
}



///////////////////////////////////////////////////////////////////////////
// InsertContainerAbove:  insert a new parent for inNode, returned in outNode,
//                   which is contructed to be of type aNodeType.  outNode becomes
//                   a child of inNode's earlier parent.
//                   Callers responsibility to make sure inNode's can be child
//                   of outNode, and outNode can be child of old parent.
nsresult
nsEditor::InsertContainerAbove( nsIDOMNode *inNode,
                                nsCOMPtr<nsIDOMNode> *outNode,
                                const nsAString &aNodeType,
                                const nsAString *aAttribute,
                                const nsAString *aValue)
{
  if (!inNode || !outNode)
    return NS_ERROR_NULL_POINTER;
  nsCOMPtr<nsIDOMNode> parent;
  PRInt32 offset;
  nsresult res = GetNodeLocation(inNode, address_of(parent), &offset);
  if (NS_FAILED(res)) return res;

  // create new container
  nsCOMPtr<nsIContent> newContent;

  //new call to use instead to get proper HTML element, bug# 39919
  res = CreateHTMLContent(aNodeType, getter_AddRefs(newContent));
  nsCOMPtr<nsIDOMElement> elem = do_QueryInterface(newContent);
  if (NS_FAILED(res)) return res;
  *outNode = do_QueryInterface(elem);

  // set attribute if needed
  if (aAttribute && aValue && !aAttribute->IsEmpty())
  {
    res = elem->SetAttribute(*aAttribute, *aValue);
    if (NS_FAILED(res)) return res;
  }

  // notify our internal selection state listener
  nsAutoInsertContainerSelNotify selNotify(mRangeUpdater);

  // put inNode in new parent, outNode
  res = DeleteNode(inNode);
  if (NS_FAILED(res)) return res;

  {
    nsAutoTxnsConserveSelection conserveSelection(this);
    res = InsertNode(inNode, *outNode, 0);
    if (NS_FAILED(res)) return res;
  }

  // put new parent in doc
  nsCOMPtr<nsIHTMLEditor> htmlEditor;
  nsCOMPtr<nsIEditor> editor = this;
  htmlEditor = do_QueryInterface(editor);
  if (!htmlEditor) return InsertNode(*outNode, parent, offset);
  return htmlEditor->InsertNodeAtPoint(*outNode, (nsIDOMNode **)address_of(parent), &offset, true);
}

nsresult
nsEditor::InsertContainerAboveNS( nsIDOMNode *inNode,
                                nsCOMPtr<nsIDOMNode> *outNode,
                                const nsAString &aNodeType,
                                nsIAtom * atomNS,
                                const nsAString *aAttribute,
                                const nsAString *aValue
                                )
{
  if (!inNode || !outNode)
    return NS_ERROR_NULL_POINTER;
  nsCOMPtr<nsIDOMNode> parent;
  PRInt32 offset;
  nsresult res = GetNodeLocation(inNode, address_of(parent), &offset);
  if (NS_FAILED(res)) return res;

  // create new container
  nsCOMPtr<nsIDOMElement> newElement;

  nsAutoString strQualifiedName;
  nsAutoString strPrefix;
  nsCOMPtr<nsIDOM3Node> node;
  node = do_QueryInterface(inNode);
  nsAutoString strNS;
  if (atomNS)
  {
    atomNS->ToString(strNS);
    node->LookupPrefix(strNS,strPrefix);
  }
  if (strPrefix.Length()>0) strQualifiedName = strPrefix + NS_LITERAL_STRING(":") + aNodeType;
  else strQualifiedName = aNodeType;
  res = CreateContentNS(strQualifiedName, atomNS, getter_AddRefs(newElement));
  if (aAttribute && aValue) res = newElement->SetAttribute(*aAttribute, *aValue);
  *outNode = do_QueryInterface(newElement);

  // set attribute if needed
//  if (aAttribute && aValue && !aAttribute->IsEmpty())
//  {
//    res = elem->SetAttribute(*aAttribute, *aValue);
//    if (NS_FAILED(res)) return res;
//  }

  // notify our internal selection state listener
  nsAutoInsertContainerSelNotify selNotify(mRangeUpdater);

  // put inNode in new parent, outNode
  res = DeleteNode(inNode);
  if (NS_FAILED(res)) return res;

  {
    nsAutoTxnsConserveSelection conserveSelection(this);
    res = InsertNode(inNode, *outNode, 0);
    if (NS_FAILED(res)) return res;
  }

  // put new parent in doc
  nsCOMPtr<nsIHTMLEditor> htmlEditor;
  nsCOMPtr<nsIEditor> editor = this;
  htmlEditor = do_QueryInterface(editor);
  if (!htmlEditor) return InsertNode(*outNode, parent, offset);
  return htmlEditor->InsertNodeAtPoint(*outNode, (nsIDOMNode **)address_of(parent), &offset, true);
}

///////////////////////////////////////////////////////////////////////////
// MoveNode:  move aNode to {aParent,aOffset}
nsresult
nsEditor::MoveNode(nsIDOMNode *aNode, nsIDOMNode *aParent, PRInt32 aOffset)
{
  if (!aNode || !aParent)
    return NS_ERROR_NULL_POINTER;

  nsCOMPtr<nsIDOMNode> oldParent;
  PRInt32 oldOffset;
  nsresult res = GetNodeLocation(aNode, address_of(oldParent), &oldOffset);

  if (aOffset == -1)
  {
    PRUint32 unsignedOffset;
    // magic value meaning "move to end of aParent"
    res = GetLengthOfDOMNode(aParent, unsignedOffset);
    if (NS_FAILED(res)) return res;
    aOffset = (PRInt32)unsignedOffset;
  }

  // don't do anything if it's already in right place
  if ((aParent == oldParent.get()) && (oldOffset == aOffset)) return NS_OK;

  // notify our internal selection state listener
  nsAutoMoveNodeSelNotify selNotify(mRangeUpdater, oldParent, oldOffset, aParent, aOffset);

  // need to adjust aOffset if we are moving aNode further along in its current parent
  if ((aParent == oldParent.get()) && (oldOffset < aOffset))
  {
    aOffset--;  // this is because when we delete aNode, it will make the offsets after it off by one
  }

  // put aNode in new parent
  res = DeleteNode(aNode);
  if (NS_FAILED(res)) return res;
  return InsertNode(aNode, aParent, aOffset);
}


NS_IMETHODIMP
nsEditor::AddEditorObserver(nsIEditorObserver *aObserver)
{
  // we don't keep ownership of the observers.  They must
  // remove themselves as observers before they are destroyed.

  if (!aObserver)
    return NS_ERROR_NULL_POINTER;

  // Make sure the listener isn't already on the list
  if (mEditorObservers.IndexOf(aObserver) == -1)
  {
    if (!mEditorObservers.AppendObject(aObserver))
      return NS_ERROR_FAILURE;
  }

  return NS_OK;
}


NS_IMETHODIMP
nsEditor::RemoveEditorObserver(nsIEditorObserver *aObserver)
{
  if (!aObserver)
    return NS_ERROR_FAILURE;

  if (!mEditorObservers.RemoveObject(aObserver))
    return NS_ERROR_FAILURE;

  return NS_OK;
}

void nsEditor::NotifyEditorObservers(void)
{
  for (PRInt32 i = 0; i < mEditorObservers.Count(); i++)
    mEditorObservers[i]->EditAction();
}


NS_IMETHODIMP
nsEditor::AddEditActionListener(nsIEditActionListener *aListener)
{
  if (!aListener)
    return NS_ERROR_NULL_POINTER;

  // Make sure the listener isn't already on the list
  if (mActionListeners.IndexOf(aListener) == -1)
  {
    if (!mActionListeners.AppendObject(aListener))
      return NS_ERROR_FAILURE;
  }

  return NS_OK;
}


NS_IMETHODIMP
nsEditor::RemoveEditActionListener(nsIEditActionListener *aListener)
{
  if (!aListener)
    return NS_ERROR_FAILURE;

  if (!mActionListeners.RemoveObject(aListener))
    return NS_ERROR_FAILURE;

  return NS_OK;
}




NS_IMETHODIMP
nsEditor::AddDocumentStateListener(nsIDocumentStateListener *aListener)
{
  if (!aListener)
    return NS_ERROR_NULL_POINTER;

  if (mDocStateListeners.IndexOf(aListener) == -1)
  {
    if (!mDocStateListeners.AppendObject(aListener))
      return NS_ERROR_FAILURE;
  }

  return NS_OK;
}


NS_IMETHODIMP
nsEditor::RemoveDocumentStateListener(nsIDocumentStateListener *aListener)
{
  if (!aListener)
    return NS_ERROR_NULL_POINTER;

  if (!mDocStateListeners.RemoveObject(aListener))
    return NS_ERROR_FAILURE;

  return NS_OK;
}



NS_IMETHODIMP nsEditor::OutputToString(const nsAString& aFormatType,
                                       PRUint32 aFlags,
                                       nsAString& aOutputString)
{
  // these should be implemented by derived classes.
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsEditor::OutputToStream(nsIOutputStream* aOutputStream,
                         const nsAString& aFormatType,
                         const nsACString& aCharsetOverride,
                         PRUint32 aFlags)
{
  // these should be implemented by derived classes.
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsEditor::DumpContentTree()
{
#ifdef DEBUG
  nsCOMPtr<nsIContent> root = do_QueryInterface(mRootElement);
  if (root)  root->List(stdout);
#endif
  return NS_OK;
}


NS_IMETHODIMP
nsEditor::DebugDumpContent()
{
#ifdef DEBUG
  nsCOMPtr<nsIDOMHTMLDocument> doc = do_QueryReferent(mDocWeak);
  if (!doc) return NS_ERROR_NOT_INITIALIZED;

  nsCOMPtr<nsIDOMHTMLElement>bodyElem;
  doc->GetBody(getter_AddRefs(bodyElem));
  nsCOMPtr<nsIContent> content = do_QueryInterface(bodyElem);
  if (content)
    content->List();
#endif
  return NS_OK;
}


NS_IMETHODIMP
nsEditor::DebugUnitTests(PRInt32 *outNumTests, PRInt32 *outNumTestsFailed)
{
#ifdef DEBUG
  NS_NOTREACHED("This should never get called. Overridden by subclasses");
#endif
  return NS_OK;
}


PRBool
nsEditor::ArePreservingSelection()
{
  return !(mSavedSel.IsEmpty());
}

nsresult
nsEditor::PreserveSelectionAcrossActions(nsISelection *aSel)
{
  mSavedSel.SaveSelection(aSel);
  mRangeUpdater.RegisterSelectionState(mSavedSel);
  return NS_OK;
}

nsresult
nsEditor::RestorePreservedSelection(nsISelection *aSel)
{
  if (mSavedSel.IsEmpty()) return NS_ERROR_FAILURE;
  mSavedSel.RestoreSelection(aSel);
  StopPreservingSelection();
  return NS_OK;
}

void
nsEditor::StopPreservingSelection()
{
  mRangeUpdater.DropSelectionState(mSavedSel);
  mSavedSel.MakeEmpty();
}



//
// The BeingComposition method is called from the Editor Composition event listeners.
//
NS_IMETHODIMP
nsEditor::QueryComposition(nsTextEventReply* aReply)
{
  nsresult result;
  nsCOMPtr<nsISelection> selection;
  nsCOMPtr<nsISelectionController> selcon = do_QueryReferent(mSelConWeak);
  if (selcon)
    selcon->GetSelection(nsISelectionController::SELECTION_NORMAL, getter_AddRefs(selection));

  if (!mPresShellWeak) return NS_ERROR_NOT_INITIALIZED;
  nsCOMPtr<nsIPresShell> ps = do_QueryReferent(mPresShellWeak);
  if (!ps) return NS_ERROR_NOT_INITIALIZED;
  nsCOMPtr<nsICaret> caretP;
  result = ps->GetCaret(getter_AddRefs(caretP));

  if (NS_SUCCEEDED(result) && caretP) {
    if (aReply) {
      caretP->SetCaretDOMSelection(selection);

      // XXX_kin: BEGIN HACK! HACK! HACK!
      // XXX_kin:
      // XXX_kin: This is lame! The IME stuff needs caret coordinates
      // XXX_kin: synchronously, but the editor could be using async
      // XXX_kin: updates (reflows and paints) for performance reasons.
      // XXX_kin: In order to give IME what it needs, we have to temporarily
      // XXX_kin: switch to sync updating during this call so that the
      // XXX_kin: nsAutoUpdateViewBatch can force sync reflows and paints
      // XXX_kin: so that we get back accurate caret coordinates.

      PRUint32 flags = 0;

      if (NS_SUCCEEDED(GetFlags(&flags)) &&
          (flags & nsIPlaintextEditor::eEditorUseAsyncUpdatesMask))
      {
        PRBool restoreFlags = PR_FALSE;

        if (NS_SUCCEEDED(SetFlags(flags & (~nsIPlaintextEditor::eEditorUseAsyncUpdatesMask))))
        {
           // Scope the viewBatch within this |if| block so that we
           // force synchronous reflows and paints before restoring
           // our editor flags below.

           nsAutoUpdateViewBatch viewBatch(this);
           restoreFlags = PR_TRUE;
        }

        // Restore the previous set of flags!

        if (restoreFlags)
          SetFlags(flags);
      }
      // XXX_kin: END HACK! HACK! HACK!
      result = caretP->GetCaretCoordinates(nsICaret::eIMECoordinates, selection,
		                      &(aReply->mCursorPosition), &(aReply->mCursorIsCollapsed), nsnull);
    }
  }
  return result;
}
NS_IMETHODIMP
nsEditor::BeginComposition(nsTextEventReply* aReply)
{
#ifdef DEBUG_tague
  printf("nsEditor::StartComposition\n");
#endif
  nsresult ret = QueryComposition(aReply);
  mInIMEMode = PR_TRUE;
  if (mPhonetic)
    mPhonetic->Truncate(0);

  return ret;
}

NS_IMETHODIMP
nsEditor::EndComposition(void)
{
  if (!mInIMEMode) return NS_OK; // nothing to do

  nsresult result = NS_OK;

  // commit the IME transaction..we can get at it via the transaction mgr.
  // Note that this means IME won't work without an undo stack!
  if (mTxnMgr)
  {
    nsCOMPtr<nsITransaction> txn;
    result = mTxnMgr->PeekUndoStack(getter_AddRefs(txn));
    nsCOMPtr<nsIAbsorbingTransaction> plcTxn = do_QueryInterface(txn);
    if (plcTxn)
    {
      result = plcTxn->Commit();
    }
  }

  /* reset the data we need to construct a transaction */
  mIMETextNode = do_QueryInterface(nsnull);
  mIMETextOffset = 0;
  mIMEBufferLength = 0;
  mInIMEMode = PR_FALSE;
  mIsIMEComposing = PR_FALSE;

  // notify editor observers of action
  NotifyEditorObservers();

  return result;
}

NS_IMETHODIMP
nsEditor::SetCompositionString(const nsAString& aCompositionString, nsIPrivateTextRangeList* aTextRangeList,nsTextEventReply* aReply)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsEditor::GetPhonetic(nsAString& aPhonetic)
{
  if (mPhonetic)
    aPhonetic = *mPhonetic;
  else
    aPhonetic.Truncate(0);

  return NS_OK;
}


static nsresult
GetEditorContentWindow(nsIPresShell *aPresShell, nsIDOMElement *aRoot, nsIWidget **aResult)
{
  if (!aPresShell || !aRoot || !aResult)
    return NS_ERROR_NULL_POINTER;

  *aResult = 0;

  nsCOMPtr<nsIContent> content = do_QueryInterface(aRoot);

  if (!content)
    return NS_ERROR_FAILURE;

  // Not ref counted
  nsIFrame *frame = aPresShell->GetPrimaryFrameFor(content);

  if (!frame)
    return NS_ERROR_FAILURE;

  *aResult = frame->GetWindow();
  if (!*aResult)
    return NS_ERROR_FAILURE;

  NS_ADDREF(*aResult);
  return NS_OK;
}

nsresult
nsEditor::GetKBStateControl(nsIKBStateControl **aKBSC)
{
  if (!aKBSC)
    return NS_ERROR_NULL_POINTER;
  *aKBSC = nsnull;
  nsCOMPtr<nsIPresShell> shell;
  nsresult res = GetPresShell(getter_AddRefs(shell));

  if (NS_FAILED(res))
    return res;

  if (!shell)
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsIWidget> widget;
  res = GetEditorContentWindow(shell, GetRoot(), getter_AddRefs(widget));
  if (NS_FAILED(res))
    return res;

  nsCOMPtr<nsIKBStateControl> kb = do_QueryInterface(widget);
  if (!kb)
    return NS_ERROR_NOT_INITIALIZED;

  NS_ADDREF(*aKBSC = kb);

  return NS_OK;
}

NS_IMETHODIMP
nsEditor::ForceCompositionEnd()
{

// We can test mInIMEMode and do some optimization for Mac and Window
// Howerver, since UNIX support over-the-spot, we cannot rely on that
// flag for Unix.
// We should use nsILookAndFeel to resolve this

#if defined(XP_MACOSX) || defined(XP_WIN) || defined(XP_OS2)
  if(! mInIMEMode)
    return NS_OK;
#endif

#ifdef XP_UNIX
  if(mFlags & nsIPlaintextEditor::eEditorPasswordMask)
	return NS_OK;
#endif

  nsCOMPtr<nsIKBStateControl> kb;
  nsresult res = GetKBStateControl(getter_AddRefs(kb));
  if (NS_FAILED(res))
    return res;

  if (kb) {
    res = kb->ResetInputState();
    if (NS_FAILED(res))
      return res;
  }

  return NS_OK;
}

NS_IMETHODIMP
nsEditor::NotifyIMEOnFocus()
{
  return NS_OK;
}

NS_IMETHODIMP
nsEditor::NotifyIMEOnBlur()
{
  return NS_OK;
}

NS_IMETHODIMP
nsEditor::GetPreferredIMEState(PRUint32 *aState)
{
  NS_ENSURE_ARG_POINTER(aState);
  *aState = nsIContent::IME_STATUS_ENABLE;

  PRUint32 flags;
  nsresult rv = GetFlags(&flags);
  NS_ENSURE_SUCCESS(rv, rv);

  if (flags & (nsIPlaintextEditor::eEditorReadonlyMask |
               nsIPlaintextEditor::eEditorDisabledMask)) {
    *aState = nsIContent::IME_STATUS_DISABLE;
    return NS_OK;
  }

  nsCOMPtr<nsIPresShell> presShell;
  rv = GetPresShell(getter_AddRefs(presShell));
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIContent> content = do_QueryInterface(GetRoot());
  NS_ENSURE_TRUE(content, NS_ERROR_FAILURE);

  nsIFrame* frame = presShell->GetPrimaryFrameFor(content);
  NS_ENSURE_TRUE(frame, NS_ERROR_FAILURE);

  switch (frame->GetStyleUIReset()->mIMEMode) {
    case NS_STYLE_IME_MODE_AUTO:
      if (flags & (nsIPlaintextEditor::eEditorPasswordMask))
        *aState = nsIContent::IME_STATUS_PASSWORD;
      break;
    case NS_STYLE_IME_MODE_DISABLED:
      // we should use password state for |ime-mode: disabled;|.
      *aState = nsIContent::IME_STATUS_PASSWORD;
      break;
    case NS_STYLE_IME_MODE_ACTIVE:
      *aState |= nsIContent::IME_STATUS_OPEN;
      break;
    case NS_STYLE_IME_MODE_INACTIVE:
      *aState |= nsIContent::IME_STATUS_CLOSE;
      break;
  }

  return NS_OK;
}

NS_IMETHODIMP
nsEditor::GetComposing(PRBool* aResult)
{
  NS_ENSURE_ARG_POINTER(aResult);
  *aResult = IsIMEComposing();
  return NS_OK;
}

NS_IMETHODIMP
nsEditor::GetReconversionString(nsReconversionEventReply* aReply)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsEditor::GetQueryCaretRect(nsQueryCaretRectEventReply* aReply)
{
  nsCOMPtr<nsISelection> selection;
  nsresult rv = GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(rv))
    return rv;

  if (!mPresShellWeak)
    return NS_ERROR_NOT_INITIALIZED;

  nsCOMPtr<nsIPresShell> ps = do_QueryReferent(mPresShellWeak);
  if (!ps)
    return NS_ERROR_NOT_INITIALIZED;

  nsCOMPtr<nsICaret> caretP;
  rv = ps->GetCaret(getter_AddRefs(caretP));

  if (NS_FAILED(rv) || !caretP)
    return rv;

  PRBool cursorIsCollapsed;
  rv = caretP->GetCaretCoordinates(nsICaret::eIMECoordinates, selection,
                                   &aReply->mCaretRect, &cursorIsCollapsed, nsnull);
  if (NS_SUCCEEDED(rv))
    aReply->mRectIsValid = PR_TRUE;
  return rv;
}

/* Non-interface, public methods */


void
nsEditor::ContentAppended(nsIDocument *aDocument, nsIContent* aContainer,
                          PRInt32 aNewIndexInContainer)
{
  ContentInserted(aDocument, aContainer, nsnull, aNewIndexInContainer);
}

void
nsEditor::ContentInserted(nsIDocument *aDocument, nsIContent* aContainer,
                          nsIContent* aChild, PRInt32 aIndexInContainer)
{
  // XXX If we need aChild then nsEditor::ContentAppended should start passing
  //     in the child.
  if (!mRootElement)
  {
    // Need to remove the event listeners first because BeginningOfDocument
    // could set a new root (and event target) and we won't be able to remove
    // them from the old event target then.
    RemoveEventListeners();
    BeginningOfDocument();
    InstallEventListeners();
    SyncRealTimeSpell();
  }
}

void
nsEditor::ContentRemoved(nsIDocument *aDocument, nsIContent* aContainer,
                         nsIContent* aChild, PRInt32 aIndexInContainer)
{
  nsCOMPtr<nsIDOMHTMLElement> elem = do_QueryInterface(aChild);
  if (elem == mRootElement)
  {
    RemoveEventListeners();
    mRootElement = nsnull;
    mEventTarget = nsnull;
    InstallEventListeners();
  }
}

NS_IMETHODIMP
nsEditor::GetRootElement(nsIDOMElement **aRootElement)
{
  PRBool isXML = PR_FALSE;
  nsresult result;
  if (!aRootElement)
    return NS_ERROR_NULL_POINTER;

  if (mRootElement)
  {
    // if we have cached the body element, use that
    *aRootElement = mRootElement;
    NS_ADDREF(*aRootElement);
    return NS_OK;
  }

  *aRootElement = 0;

  NS_PRECONDITION(mDocWeak, "bad state, null mDocWeak");
  nsCOMPtr<nsIDOMXMLDocument> xmldoc;
  nsCOMPtr<nsIDOMHTMLDocument> doc = do_QueryReferent(mDocWeak);
  if (!doc)
  {
    xmldoc = do_QueryReferent(mDocWeak);
    isXML = PR_TRUE;
    if (!xmldoc)
    return NS_ERROR_NOT_INITIALIZED;
  }
  if (xmldoc)
  {
    nsCOMPtr<nsIDOMElement> docElement;
    result = xmldoc->GetDocumentElement(getter_AddRefs(docElement));
    if (NS_FAILED(result))
      return result;
    if (!docElement)
      return NS_ERROR_NULL_POINTER;

  mRootElement = docElement;
  *aRootElement = docElement;
}
else
{
  // Use the documents body element as the editor root if we didn't
  // get a root element during initialization.

  nsCOMPtr<nsIDOMHTMLElement> bodyElement;
  result = doc->GetBody(getter_AddRefs(bodyElement));
  if (NS_FAILED(result))
    return result;

  if (!bodyElement)
    return NS_ERROR_NULL_POINTER;

  mRootElement = bodyElement;
  *aRootElement = bodyElement;
  }
  NS_ADDREF(*aRootElement);

  return NS_OK;
}


/** All editor operations which alter the doc should be prefaced
 *  with a call to StartOperation, naming the action and direction */
NS_IMETHODIMP
nsEditor::StartOperation(PRInt32 opID, nsIEditor::EDirection aDirection)
{
  mAction = opID;
  mDirection = aDirection;
  return NS_OK;
}


/** All editor operations which alter the doc should be followed
 *  with a call to EndOperation */
NS_IMETHODIMP
nsEditor::EndOperation()
{
  mAction = nsnull;
  mDirection = eNone;
  return NS_OK;
}

NS_IMETHODIMP
nsEditor::CloneAttribute(const nsAString & aAttribute,
                         nsIDOMNode *aDestNode, nsIDOMNode *aSourceNode)
{
  if (!aDestNode || !aSourceNode)
    return NS_ERROR_NULL_POINTER;

  nsCOMPtr<nsIDOMElement> destElement = do_QueryInterface(aDestNode);
  nsCOMPtr<nsIDOMElement> sourceElement = do_QueryInterface(aSourceNode);
  if (!destElement || !sourceElement)
    return NS_ERROR_NO_INTERFACE;

  nsAutoString attrValue;
  PRBool isAttrSet;
  nsresult rv = GetAttributeValue(sourceElement,
                                  aAttribute,
                                  attrValue,
                                  &isAttrSet);
  if (NS_FAILED(rv))
    return rv;
  if (isAttrSet)
    rv = SetAttribute(destElement, aAttribute, attrValue);
  else
    rv = RemoveAttribute(destElement, aAttribute);

  return rv;
}

// Objects must be DOM elements
NS_IMETHODIMP
nsEditor::CloneAttributes(nsIDOMNode *aDestNode, nsIDOMNode *aSourceNode)
{
  if (!aDestNode || !aSourceNode)
    return NS_ERROR_NULL_POINTER;

  nsCOMPtr<nsIDOMElement> destElement = do_QueryInterface(aDestNode);
  nsCOMPtr<nsIDOMElement> sourceElement = do_QueryInterface(aSourceNode);
  if (!destElement || !sourceElement)
    return NS_ERROR_NO_INTERFACE;

  nsCOMPtr<nsIDOMNamedNodeMap> sourceAttributes;
  sourceElement->GetAttributes(getter_AddRefs(sourceAttributes));
  nsCOMPtr<nsIDOMNamedNodeMap> destAttributes;
  destElement->GetAttributes(getter_AddRefs(destAttributes));
  if (!sourceAttributes || !destAttributes)
    return NS_ERROR_FAILURE;

  nsAutoEditBatch beginBatching(this);

  // Use transaction system for undo only if destination
  //   is already in the document
  nsIDOMElement *rootElement = GetRoot();
  if (!rootElement)
    return NS_ERROR_NULL_POINTER;

  PRBool destInBody = PR_TRUE;
  nsCOMPtr<nsIDOMNode> rootNode = do_QueryInterface(rootElement);
  nsCOMPtr<nsIDOMNode> p = aDestNode;
  while (p && p != rootNode)
  {
    nsCOMPtr<nsIDOMNode> tmp;
    if (NS_FAILED(p->GetParentNode(getter_AddRefs(tmp))) || !tmp)
    {
      destInBody = PR_FALSE;
      break;
    }
    p = tmp;
  }

  PRUint32 sourceCount;
  sourceAttributes->GetLength(&sourceCount);
  PRUint32 i, destCount;
  destAttributes->GetLength(&destCount);
  nsCOMPtr<nsIDOMNode> attrNode;

  // Clear existing attributes
  for (i = 0; i < destCount; i++)
  {
    // always remove item number 0 (first item in list)
    if( NS_SUCCEEDED(destAttributes->Item(0, getter_AddRefs(attrNode))) && attrNode)
    {
      nsCOMPtr<nsIDOMAttr> destAttribute = do_QueryInterface(attrNode);
      if (destAttribute)
      {
        nsAutoString str;
        if (NS_SUCCEEDED(destAttribute->GetName(str)))
        {
          if (destInBody)
            RemoveAttribute(destElement, str);
          else
            destElement->RemoveAttribute(str);
        }
      }
    }
  }

  nsresult result = NS_OK;

  // Set just the attributes that the source element has
  for (i = 0; i < sourceCount; i++)
  {
    if( NS_SUCCEEDED(sourceAttributes->Item(i, getter_AddRefs(attrNode))) && attrNode)
    {
      nsCOMPtr<nsIDOMAttr> sourceAttribute = do_QueryInterface(attrNode);
      if (sourceAttribute)
      {
        nsAutoString sourceAttrName;
        if (NS_SUCCEEDED(sourceAttribute->GetName(sourceAttrName)))
        {
          nsAutoString sourceAttrValue;
          /*
          Presence of an attribute in the named node map indicates that it was set on the
          element even if it has no value.
          */
          if (NS_SUCCEEDED(sourceAttribute->GetValue(sourceAttrValue)))
          {
            if (destInBody) {
              result = SetAttributeOrEquivalent(destElement, sourceAttrName, sourceAttrValue, PR_FALSE);
            }
            else {
              // the element is not inserted in the document yet, we don't want to put a
              // transaction on the UndoStack
              result = SetAttributeOrEquivalent(destElement, sourceAttrName, sourceAttrValue, PR_TRUE);
            }
          } else {
            // Do we ever get here?
#if DEBUG_cmanske
            printf("Attribute in sourceAttribute has empty value in nsEditor::CloneAttributes()\n");
#endif
          }
        }
      }
    }
  }
  return result;
}


NS_IMETHODIMP nsEditor::ScrollSelectionIntoView(PRBool aScrollToAnchor)
{
  nsCOMPtr<nsISelectionController> selCon;
  if (NS_SUCCEEDED(GetSelectionController(getter_AddRefs(selCon))) && selCon)
  {
    PRInt16 region = nsISelectionController::SELECTION_FOCUS_REGION;

    if (aScrollToAnchor)
      region = nsISelectionController::SELECTION_ANCHOR_REGION;

    PRBool syncScroll = PR_TRUE;
    PRUint32 flags = 0;

    if (NS_SUCCEEDED(GetFlags(&flags)))
    {
      // If the editor is relying on asynchronous reflows, we have
      // to use asynchronous requests to scroll, so that the scrolling happens
      // after reflow requests are processed.
      // XXXbz why not just always do async scroll?
      syncScroll = !(flags & nsIPlaintextEditor::eEditorUseAsyncUpdatesMask);
    }

    // After ScrollSelectionIntoView(), the pending notifications might be
    // flushed and PresShell/PresContext/Frames may be dead. See bug 418470.
    selCon->ScrollSelectionIntoView(nsISelectionController::SELECTION_NORMAL,
                                    region, syncScroll);
  }

  return NS_OK;
}

NS_IMETHODIMP nsEditor::InsertTextImpl(const nsAString& aStringToInsert,
                                          nsCOMPtr<nsIDOMNode> *aInOutNode,
                                          PRInt32 *aInOutOffset,
                                          nsIDOMDocument *aDoc)
{
  // NOTE: caller *must* have already used nsAutoTxnsConserveSelection stack-based
  // class to turn off txn selection updating.  Caller also turned on rules sniffing
  // if desired.

  if (!aInOutNode || !*aInOutNode || !aInOutOffset || !aDoc) return NS_ERROR_NULL_POINTER;
  if (!mInIMEMode && aStringToInsert.IsEmpty()) return NS_OK;
  nsCOMPtr<nsIDOMText> nodeAsText = do_QueryInterface(*aInOutNode);
  PRInt32 offset = *aInOutOffset;
  nsresult res;
  if (mInIMEMode)
  {
    if (!nodeAsText)
    {
      // create a text node
      res = aDoc->CreateTextNode(EmptyString(), getter_AddRefs(nodeAsText));
      if (NS_FAILED(res)) return res;
      if (!nodeAsText) return NS_ERROR_NULL_POINTER;
      nsCOMPtr<nsIDOMNode> newNode = do_QueryInterface(nodeAsText);
      // then we insert it into the dom tree
      res = InsertNode(newNode, *aInOutNode, offset);
      if (NS_FAILED(res)) return res;
      offset = 0;
    }
    res = InsertTextIntoTextNodeImpl(aStringToInsert, nodeAsText, offset);
    if (NS_FAILED(res)) return res;
  }
  else
  {
    if (nodeAsText)
    {
      // we are inserting text into an existing text node.
      res = InsertTextIntoTextNodeImpl(aStringToInsert, nodeAsText, offset);
      if (NS_FAILED(res)) return res;
      *aInOutOffset += aStringToInsert.Length();
    }
    else
    {
      // we are inserting text into a non-text node
      // first we have to create a textnode (this also populates it with the text)
      res = aDoc->CreateTextNode(aStringToInsert, getter_AddRefs(nodeAsText));
      if (NS_FAILED(res)) return res;
      if (!nodeAsText) return NS_ERROR_NULL_POINTER;
      nsCOMPtr<nsIDOMNode> newNode = do_QueryInterface(nodeAsText);
      // then we insert it into the dom tree
      res = InsertNode(newNode, *aInOutNode, offset);
      if (NS_FAILED(res)) return res;
      *aInOutNode = newNode;
      *aInOutOffset = aStringToInsert.Length();
    }
  }
  return res;
}


NS_IMETHODIMP nsEditor::InsertTextIntoTextNodeImpl(const nsAString& aStringToInsert,
                                                     nsIDOMCharacterData *aTextNode,
                                                     PRInt32 aOffset, PRBool suppressIME)
{
  nsRefPtr<EditTxn> txn;
  nsresult result;
  // suppressIME s used when editor must insert text, yet this text is not
  // part of current ime operation.  example: adjusting whitespace around an ime insertion.
  if (mIMETextRangeList && mInIMEMode && !suppressIME)
  {
    if (!mIMETextNode)
    {
      mIMETextNode = aTextNode;
      mIMETextOffset = aOffset;
    }
    PRUint16 len ;
    result = mIMETextRangeList->GetLength(&len);
    if (NS_SUCCEEDED(result) && len > 0)
    {
      nsCOMPtr<nsIPrivateTextRange> range;
      for (PRUint16 i = 0; i < len; i++)
      {
        result = mIMETextRangeList->Item(i, getter_AddRefs(range));
        if (NS_SUCCEEDED(result) && range)
        {
          PRUint16 type;
          result = range->GetRangeType(&type);
          if (NS_SUCCEEDED(result))
          {
            if (type == nsIPrivateTextRange::TEXTRANGE_RAWINPUT)
            {
              PRUint16 start, end;
              result = range->GetRangeStart(&start);
              if (NS_SUCCEEDED(result))
              {
                result = range->GetRangeEnd(&end);
                if (NS_SUCCEEDED(result))
                {
                  if (!mPhonetic)
                    mPhonetic = new nsString();
                  if (mPhonetic)
                  {
                    nsAutoString tmp(aStringToInsert);
                    tmp.Mid(*mPhonetic, start, end-start);
                  }
                }
              }
            } // if
          }
        } // if
      } // for
    } // if

    nsRefPtr<IMETextTxn> imeTxn;
    result = CreateTxnForIMEText(aStringToInsert, getter_AddRefs(imeTxn));
    txn = imeTxn;
  }
  else
  {
    nsRefPtr<InsertTextTxn> insertTxn;
    result = CreateTxnForInsertText(aStringToInsert, aTextNode, aOffset,
                                    getter_AddRefs(insertTxn));
    txn = insertTxn;
  }
  if (NS_FAILED(result)) return result;

  // let listeners know what's up
  PRInt32 i;
  for (i = 0; i < mActionListeners.Count(); i++)
    mActionListeners[i]->WillInsertText(aTextNode, aOffset, aStringToInsert);

  // XXX we may not need these view batches anymore.  This is handled at a higher level now I believe
  BeginUpdateViewBatch();
  result = DoTransaction(txn);
  EndUpdateViewBatch();

  mRangeUpdater.SelAdjInsertText(aTextNode, aOffset, aStringToInsert);

  // let listeners know what happened
  for (i = 0; i < mActionListeners.Count(); i++)
    mActionListeners[i]->DidInsertText(aTextNode, aOffset, aStringToInsert, result);

  // Added some cruft here for bug 43366.  Layout was crashing because we left an
  // empty text node lying around in the document.  So I delete empty text nodes
  // caused by IME.  I have to mark the IME transaction as "fixed", which means
  // that furure ime txns won't merge with it.  This is because we don't want
  // future ime txns trying to put their text into a node that is no longer in
  // the document.  This does not break undo/redo, because all these txns are
  // wrapped in a parent PlaceHolder txn, and placeholder txns are already
  // savvy to having multiple ime txns inside them.

  // delete empty ime text node if there is one
  if (mInIMEMode && mIMETextNode)
  {
    PRUint32 len;
    mIMETextNode->GetLength(&len);
    if (!len)
    {
      DeleteNode(mIMETextNode);
      mIMETextNode = nsnull;
      static_cast<IMETextTxn*>(txn.get())->MarkFixed();  // mark the ime txn "fixed"
    }
  }

  return result;
}


NS_IMETHODIMP nsEditor::SelectEntireDocument(nsISelection *aSelection)
{
  if (!aSelection) { return NS_ERROR_NULL_POINTER; }

  nsIDOMElement *rootElement = GetRoot();
  if (!rootElement) { return NS_ERROR_NOT_INITIALIZED; }

  return aSelection->SelectAllChildren(rootElement);
}


nsresult nsEditor::GetFirstEditableNode(nsIDOMNode *aRoot, nsCOMPtr<nsIDOMNode> *outFirstNode)
{
  if (!aRoot || !outFirstNode) return NS_ERROR_NULL_POINTER;
  nsresult rv = NS_OK;
  *outFirstNode = nsnull;

  nsCOMPtr<nsIDOMNode> node = GetLeftmostChild(aRoot);
  if (node && !IsEditable(node))
  {
    nsCOMPtr<nsIDOMNode> next;
    rv = GetNextNode(node, PR_TRUE, address_of(next));
    node = next;
  }

  if (node != aRoot)
    *outFirstNode = node;

  return rv;
}

#ifdef XXX_DEAD_CODE
// jfrancis wants to keep this method around for reference
nsresult nsEditor::GetLastEditableNode(nsIDOMNode *aRoot, nsCOMPtr<nsIDOMNode> *outLastNode)
{
  if (!aRoot || !outLastNode) return NS_ERROR_NULL_POINTER;
  nsresult rv = NS_OK;
  *outLastNode = nsnull;

  nsCOMPtr<nsIDOMNode> node = GetRightmostChild(aRoot);
  if (node && !IsEditable(node))
  {
    nsCOMPtr<nsIDOMNode> next;
    rv = GetPriorNode(node, PR_TRUE, address_of(next));
    node = next;
  }

  if (node != aRoot)
    *outLastNode = node;

  return rv;
}
#endif


NS_IMETHODIMP
nsEditor::NotifyDocumentListeners(TDocumentListenerNotification aNotificationType)
{
  PRInt32 numListeners = mDocStateListeners.Count();
  if (!numListeners)
    return NS_OK;    // maybe there just aren't any.

  nsresult rv = NS_OK;
  PRInt32 i;
  switch (aNotificationType)
  {
    case eDocumentCreated:
      for (i = 0; i < numListeners;i++)
      {
        rv = mDocStateListeners[i]->NotifyDocumentCreated();
        if (NS_FAILED(rv))
          break;
      }
      break;

    case eDocumentToBeDestroyed:
      for (i = 0; i < numListeners;i++)
      {
        rv = mDocStateListeners[i]->NotifyDocumentWillBeDestroyed();
        if (NS_FAILED(rv))
          break;
      }
      break;

    case eDocumentStateChanged:
      {
        PRBool docIsDirty;
        rv = GetDocumentModified(&docIsDirty);
        if (NS_FAILED(rv)) return rv;

        if (docIsDirty == mDocDirtyState)
          return NS_OK;

        mDocDirtyState = (PRInt8)docIsDirty;

        for (i = 0; i < numListeners;i++)
        {
          rv = mDocStateListeners[i]->NotifyDocumentStateChanged(mDocDirtyState);
          if (NS_FAILED(rv))
            break;
        }
      }
      break;

    default:
      NS_NOTREACHED("Unknown notification");
  }

  return rv;
}


NS_IMETHODIMP nsEditor::CreateTxnForInsertText(const nsAString & aStringToInsert,
                                               nsIDOMCharacterData *aTextNode,
                                               PRInt32 aOffset,
                                               InsertTextTxn ** aTxn)
{
  if (!aTextNode || !aTxn) return NS_ERROR_NULL_POINTER;
  nsresult result;

  result = TransactionFactory::GetNewTransaction(InsertTextTxn::GetCID(), (EditTxn **)aTxn);
  if (NS_FAILED(result)) return result;
  if (!*aTxn) return NS_ERROR_OUT_OF_MEMORY;
  result = (*aTxn)->Init(aTextNode, aOffset, aStringToInsert, this);
  return result;
}


NS_IMETHODIMP nsEditor::DeleteText(nsIDOMCharacterData *aElement,
                              PRUint32             aOffset,
                              PRUint32             aLength)
{
  nsRefPtr<DeleteTextTxn> txn;
  nsresult result = CreateTxnForDeleteText(aElement, aOffset, aLength,
                                           getter_AddRefs(txn));
  nsAutoRules beginRulesSniffing(this, kOpDeleteText, nsIEditor::ePrevious);
  if (NS_SUCCEEDED(result))
  {
    // let listeners know what's up
    PRInt32 i;
    for (i = 0; i < mActionListeners.Count(); i++)
      mActionListeners[i]->WillDeleteText(aElement, aOffset, aLength);

    result = DoTransaction(txn);

    // let listeners know what happened
    for (i = 0; i < mActionListeners.Count(); i++)
      mActionListeners[i]->DidDeleteText(aElement, aOffset, aLength, result);
  }
  return result;
}


NS_IMETHODIMP nsEditor::CreateTxnForDeleteText(nsIDOMCharacterData *aElement,
                                               PRUint32             aOffset,
                                               PRUint32             aLength,
                                               DeleteTextTxn      **aTxn)
{
  if (!aElement)
    return NS_ERROR_NULL_POINTER;

  nsresult result = TransactionFactory::GetNewTransaction(DeleteTextTxn::GetCID(), (EditTxn **)aTxn);
  if (NS_SUCCEEDED(result))  {
    result = (*aTxn)->Init(this, aElement, aOffset, aLength, &mRangeUpdater);
  }
  return result;
}




NS_IMETHODIMP nsEditor::CreateTxnForSplitNode(nsIDOMNode *aNode,
                                         PRUint32    aOffset,
                                         SplitElementTxn **aTxn)
{
  if (!aNode)
    return NS_ERROR_NULL_POINTER;

  nsresult result = TransactionFactory::GetNewTransaction(SplitElementTxn::GetCID(), (EditTxn **)aTxn);
  if (NS_FAILED(result))
    return result;

  return (*aTxn)->Init(this, aNode, aOffset);
}

NS_IMETHODIMP nsEditor::CreateTxnForJoinNode(nsIDOMNode  *aLeftNode,
                                             nsIDOMNode  *aRightNode,
                                             JoinElementTxn **aTxn)
{
  if (!aLeftNode || !aRightNode)
    return NS_ERROR_NULL_POINTER;

  nsresult result = TransactionFactory::GetNewTransaction(JoinElementTxn::GetCID(), (EditTxn **)aTxn);
  if (NS_SUCCEEDED(result))  {
    result = (*aTxn)->Init(this, aLeftNode, aRightNode);
  }
  return result;
}


// END nsEditor core implementation


// BEGIN nsEditor public helper methods

nsresult
nsEditor::SplitNodeImpl(nsIDOMNode * aExistingRightNode,
                        PRInt32      aOffset,
                        nsIDOMNode*  aNewLeftNode,
                        nsIDOMNode*  aParent)
{
#ifdef NS_DEBUG_EDITOR
  if (gNoisy) { printf("SplitNodeImpl: left=%p, right=%p, offset=%d\n", (void*)aNewLeftNode, (void*)aExistingRightNode, aOffset); }
#endif

  NS_ASSERTION(((nsnull!=aExistingRightNode) &&
                (nsnull!=aNewLeftNode) &&
                (nsnull!=aParent)),
                "null arg");
  nsresult result;
  if ((nsnull!=aExistingRightNode) &&
      (nsnull!=aNewLeftNode) &&
      (nsnull!=aParent))
  {
    // get selection
    nsCOMPtr<nsISelection> selection;
    result = GetSelection(getter_AddRefs(selection));
    if (NS_FAILED(result)) return result;
    if (!selection) return NS_ERROR_NULL_POINTER;

    // remember some selection points
    nsCOMPtr<nsIDOMNode> selStartNode, selEndNode;
    PRInt32 selStartOffset, selEndOffset;
    result = GetStartNodeAndOffset(selection, getter_AddRefs(selStartNode), &selStartOffset);
    if (NS_FAILED(result)) selStartNode = nsnull;  // if selection is cleared, remember that
    result = GetEndNodeAndOffset(selection, getter_AddRefs(selEndNode), &selEndOffset);
    if (NS_FAILED(result)) selStartNode = nsnull;  // if selection is cleared, remember that

    nsCOMPtr<nsIDOMNode> resultNode;
    result = aParent->InsertBefore(aNewLeftNode, aExistingRightNode, getter_AddRefs(resultNode));
    //printf("  after insert\n"); content->List();  // DEBUG
    if (NS_SUCCEEDED(result))
    {
      // split the children between the 2 nodes
      // at this point, aExistingRightNode has all the children
      // move all the children whose index is < aOffset to aNewLeftNode
      if (0<=aOffset) // don't bother unless we're going to move at least one child
      {
        // if it's a text node, just shuffle around some text
        nsCOMPtr<nsIDOMCharacterData> rightNodeAsText( do_QueryInterface(aExistingRightNode) );
        nsCOMPtr<nsIDOMCharacterData> leftNodeAsText( do_QueryInterface(aNewLeftNode) );
        if (leftNodeAsText && rightNodeAsText)
        {
          // fix right node
          nsAutoString leftText;
          rightNodeAsText->SubstringData(0, aOffset, leftText);
          rightNodeAsText->DeleteData(0, aOffset);
          // fix left node
          leftNodeAsText->SetData(leftText);
          // moose
        }
        else
        {  // otherwise it's an interior node, so shuffle around the children
           // go through list backwards so deletes don't interfere with the iteration
          nsCOMPtr<nsIDOMNodeList> childNodes;
          result = aExistingRightNode->GetChildNodes(getter_AddRefs(childNodes));
          if ((NS_SUCCEEDED(result)) && (childNodes))
          {
            PRInt32 i=aOffset-1;
            for ( ; ((NS_SUCCEEDED(result)) && (0<=i)); i--)
            {
              nsCOMPtr<nsIDOMNode> childNode;
              result = childNodes->Item(i, getter_AddRefs(childNode));
              if ((NS_SUCCEEDED(result)) && (childNode))
              {
                result = aExistingRightNode->RemoveChild(childNode, getter_AddRefs(resultNode));
                //printf("  after remove\n"); content->List();  // DEBUG
                if (NS_SUCCEEDED(result))
                {
                  nsCOMPtr<nsIDOMNode> firstChild;
                  aNewLeftNode->GetFirstChild(getter_AddRefs(firstChild));
                  result = aNewLeftNode->InsertBefore(childNode, firstChild, getter_AddRefs(resultNode));
                  //printf("  after append\n"); content->List();  // DEBUG
                }
              }
            }
          }
        }
        // handle selection
        if (GetShouldTxnSetSelection())
        {
          // editor wants us to set selection at split point
          selection->Collapse(aNewLeftNode, aOffset);
        }
        else if (selStartNode)
        {
          // else adjust the selection if needed.  if selStartNode is null, then there was no selection.
          // HACK: this is overly simplified - multi-range selections need more work than this
          if (selStartNode.get() == aExistingRightNode)
          {
            if (selStartOffset < aOffset)
            {
              selStartNode = aNewLeftNode;
            }
            else
            {
              selStartOffset -= aOffset;
            }
          }
          if (selEndNode.get() == aExistingRightNode)
          {
            if (selEndOffset < aOffset)
            {
              selEndNode = aNewLeftNode;
            }
            else
            {
              selEndOffset -= aOffset;
            }
          }
          selection->Collapse(selStartNode,selStartOffset);
          selection->Extend(selEndNode,selEndOffset);
        }
      }
    }
  }
  else
    result = NS_ERROR_INVALID_ARG;

  return result;
}

nsresult
nsEditor::JoinNodesImpl(nsIDOMNode * aNodeToKeep,
                        nsIDOMNode * aNodeToJoin,
                        nsIDOMNode * aParent,
                        PRBool       aNodeToKeepIsFirst)
{
  NS_ASSERTION(aNodeToKeep && aNodeToJoin && aParent, "null arg");
  nsresult result;
  if (aNodeToKeep && aNodeToJoin && aParent)
  {
    // get selection
    nsCOMPtr<nsISelection> selection;
    GetSelection(getter_AddRefs(selection));
    if (!selection) return NS_ERROR_NULL_POINTER;

    // remember some selection points
    nsCOMPtr<nsIDOMNode> selStartNode, selEndNode;
    PRInt32 selStartOffset, selEndOffset, joinOffset, keepOffset;
    result = GetStartNodeAndOffset(selection, getter_AddRefs(selStartNode), &selStartOffset);
    if (NS_FAILED(result)) selStartNode = nsnull;
    result = GetEndNodeAndOffset(selection, getter_AddRefs(selEndNode), &selEndOffset);
    // Joe or Kin should comment here on why the following line is not a copy/paste error
    if (NS_FAILED(result)) selStartNode = nsnull;

    nsCOMPtr<nsIDOMNode> leftNode;
    if (aNodeToKeepIsFirst)
      leftNode = aNodeToKeep;
    else
      leftNode = aNodeToJoin;

    PRUint32 firstNodeLength;
    result = GetLengthOfDOMNode(leftNode, firstNodeLength);
    if (NS_FAILED(result)) return result;
    nsCOMPtr<nsIDOMNode> parent;
    result = GetNodeLocation(aNodeToJoin, address_of(parent), &joinOffset);
    if (NS_FAILED(result)) return result;
    result = GetNodeLocation(aNodeToKeep, address_of(parent), &keepOffset);
    if (NS_FAILED(result)) return result;

    // if selection endpoint is between the nodes, remember it as being
    // in the one that is going away instead.  This simplifies later selection
    // adjustment logic at end of this method.
    if (selStartNode)
    {
      if (selStartNode == parent)
      {
        if (aNodeToKeepIsFirst)
        {
          if ((selStartOffset > keepOffset) && (selStartOffset <= joinOffset))
          {
            selStartNode = aNodeToJoin;
            selStartOffset = 0;
          }
        }
        else
        {
          if ((selStartOffset > joinOffset) && (selStartOffset <= keepOffset))
          {
            selStartNode = aNodeToJoin;
            selStartOffset = firstNodeLength;
          }
        }
      }
      if (selEndNode == parent)
      {
        if (aNodeToKeepIsFirst)
        {
          if ((selEndOffset > keepOffset) && (selEndOffset <= joinOffset))
          {
            selEndNode = aNodeToJoin;
            selEndOffset = 0;
          }
        }
        else
        {
          if ((selEndOffset > joinOffset) && (selEndOffset <= keepOffset))
          {
            selEndNode = aNodeToJoin;
            selEndOffset = firstNodeLength;
          }
        }
      }
    }
    // ok, ready to do join now.
    // if it's a text node, just shuffle around some text
    nsCOMPtr<nsIDOMCharacterData> keepNodeAsText( do_QueryInterface(aNodeToKeep) );
    nsCOMPtr<nsIDOMCharacterData> joinNodeAsText( do_QueryInterface(aNodeToJoin) );
    if (keepNodeAsText && joinNodeAsText)
    {
      nsAutoString rightText;
      nsAutoString leftText;
      if (aNodeToKeepIsFirst)
      {
        keepNodeAsText->GetData(leftText);
        joinNodeAsText->GetData(rightText);
      }
      else
      {
        keepNodeAsText->GetData(rightText);
        joinNodeAsText->GetData(leftText);
      }
      leftText += rightText;
      keepNodeAsText->SetData(leftText);
    }
    else
    {  // otherwise it's an interior node, so shuffle around the children
      nsCOMPtr<nsIDOMNodeList> childNodes;
      result = aNodeToJoin->GetChildNodes(getter_AddRefs(childNodes));
      if ((NS_SUCCEEDED(result)) && (childNodes))
      {
        PRInt32 i;  // must be signed int!
        PRUint32 childCount=0;
        nsCOMPtr<nsIDOMNode> firstNode; //only used if aNodeToKeepIsFirst is false
        childNodes->GetLength(&childCount);
        if (!aNodeToKeepIsFirst)
        { // remember the first child in aNodeToKeep, we'll insert all the children of aNodeToJoin in front of it
          result = aNodeToKeep->GetFirstChild(getter_AddRefs(firstNode));
          // GetFirstChild returns nsnull firstNode if aNodeToKeep has no children, that's ok.
        }
        nsCOMPtr<nsIDOMNode> resultNode;
        // have to go through the list backwards to keep deletes from interfering with iteration
        nsCOMPtr<nsIDOMNode> previousChild;
        for (i=childCount-1; ((NS_SUCCEEDED(result)) && (0<=i)); i--)
        {
          nsCOMPtr<nsIDOMNode> childNode;
          result = childNodes->Item(i, getter_AddRefs(childNode));
          if ((NS_SUCCEEDED(result)) && (childNode))
          {
            if (aNodeToKeepIsFirst)
            { // append children of aNodeToJoin
              //was result = aNodeToKeep->AppendChild(childNode, getter_AddRefs(resultNode));
              result = aNodeToKeep->InsertBefore(childNode, previousChild, getter_AddRefs(resultNode));
              previousChild = do_QueryInterface(childNode);
            }
            else
            { // prepend children of aNodeToJoin
              result = aNodeToKeep->InsertBefore(childNode, firstNode, getter_AddRefs(resultNode));
              firstNode = do_QueryInterface(childNode);
            }
          }
        }
      }
      else if (!childNodes) {
        result = NS_ERROR_NULL_POINTER;
      }
    }
    if (NS_SUCCEEDED(result))
    { // delete the extra node
      nsCOMPtr<nsIDOMNode> resultNode;
      result = aParent->RemoveChild(aNodeToJoin, getter_AddRefs(resultNode));

      if (GetShouldTxnSetSelection())
      {
        // editor wants us to set selection at join point
        selection->Collapse(aNodeToKeep, firstNodeLength);
      }
      else if (selStartNode)
      {
        // and adjust the selection if needed
        // HACK: this is overly simplified - multi-range selections need more work than this
        PRBool bNeedToAdjust = PR_FALSE;

        // check to see if we joined nodes where selection starts
        if (selStartNode.get() == aNodeToJoin)
        {
          bNeedToAdjust = PR_TRUE;
          selStartNode = aNodeToKeep;
          if (aNodeToKeepIsFirst)
          {
            selStartOffset += firstNodeLength;
          }
        }
        else if ((selStartNode.get() == aNodeToKeep) && !aNodeToKeepIsFirst)
        {
          bNeedToAdjust = PR_TRUE;
          selStartOffset += firstNodeLength;
        }

        // check to see if we joined nodes where selection ends
        if (selEndNode.get() == aNodeToJoin)
        {
          bNeedToAdjust = PR_TRUE;
          selEndNode = aNodeToKeep;
          if (aNodeToKeepIsFirst)
          {
            selEndOffset += firstNodeLength;
          }
        }
        else if ((selEndNode.get() == aNodeToKeep) && !aNodeToKeepIsFirst)
        {
          bNeedToAdjust = PR_TRUE;
          selEndOffset += firstNodeLength;
        }

        // adjust selection if needed
        if (bNeedToAdjust)
        {
          selection->Collapse(selStartNode,selStartOffset);
          selection->Extend(selEndNode,selEndOffset);
        }
      }
    }
  }
  else
    result = NS_ERROR_INVALID_ARG;

  return result;
}


nsresult
nsEditor::GetChildOffset(nsIDOMNode *aChild, nsIDOMNode *aParent, PRInt32 &aOffset)
{
  NS_ASSERTION((aChild && aParent), "bad args");

  nsCOMPtr<nsIContent> content = do_QueryInterface(aParent);
  nsCOMPtr<nsIContent> cChild = do_QueryInterface(aChild);
  if (!cChild || !content)
    return NS_ERROR_NULL_POINTER;

  aOffset = content->IndexOf(cChild);

  return NS_OK;
}

nsresult
nsEditor::GetNodeLocation(nsIDOMNode *inChild, nsCOMPtr<nsIDOMNode> *outParent, PRInt32 *outOffset)
{
  NS_ASSERTION((inChild && outParent && outOffset), "bad args");
  nsresult result = NS_ERROR_NULL_POINTER;
  if (inChild && outParent && outOffset)
  {
    result = inChild->GetParentNode(getter_AddRefs(*outParent));
    if ((NS_SUCCEEDED(result)) && (*outParent))
    {
      result = GetChildOffset(inChild, *outParent, *outOffset);
    }
  }
  return result;
}

// returns the number of things inside aNode.
// If aNode is text, returns number of characters. If not, returns number of children nodes.
nsresult
nsEditor::GetLengthOfDOMNode(nsIDOMNode *aNode, PRUint32 &aCount)
{
  aCount = 0;
  if (!aNode) { return NS_ERROR_NULL_POINTER; }
  nsresult result=NS_OK;
  nsCOMPtr<nsIDOMCharacterData>nodeAsChar = do_QueryInterface(aNode);
  if (nodeAsChar) {
    nodeAsChar->GetLength(&aCount);
  }
  else
  {
    PRBool hasChildNodes;
    aNode->HasChildNodes(&hasChildNodes);
    if (hasChildNodes)
    {
      nsCOMPtr<nsIDOMNodeList>nodeList;
      result = aNode->GetChildNodes(getter_AddRefs(nodeList));
      if (NS_SUCCEEDED(result) && nodeList) {
        nodeList->GetLength(&aCount);
      }
    }
  }
  return result;
}


nsresult
nsEditor::GetPriorNode(nsIDOMNode  *aParentNode,
                       PRInt32      aOffset,
                       PRBool       aEditableNode,
                       nsCOMPtr<nsIDOMNode> *aResultNode,
                       PRBool       bNoBlockCrossing)
{
  // just another version of GetPriorNode that takes a {parent, offset}
  // instead of a node
  if (!aParentNode || !aResultNode) { return NS_ERROR_NULL_POINTER; }
  *aResultNode = nsnull;

  // if we are at beginning of node, or it is a textnode, then just look before it
  if (!aOffset || IsTextNode(aParentNode))
  {
    if (bNoBlockCrossing && IsBlockNode(aParentNode))
    {
      // if we aren't allowed to cross blocks, don't look before this block
      return NS_OK;
    }
    return GetPriorNode(aParentNode, aEditableNode, aResultNode, bNoBlockCrossing);
  }

  // else look before the child at 'aOffset'
  nsCOMPtr<nsIDOMNode> child = GetChildAt(aParentNode, aOffset);
  if (child)
    return GetPriorNode(child, aEditableNode, aResultNode, bNoBlockCrossing);

  // unless there isn't one, in which case we are at the end of the node
  // and want the deep-right child.
  *aResultNode = GetRightmostChild(aParentNode, bNoBlockCrossing);
  if (!*aResultNode || !aEditableNode || IsEditable(*aResultNode))
    return NS_OK;

  // restart the search from the non-editable node we just found
  nsCOMPtr<nsIDOMNode> notEditableNode = do_QueryInterface(*aResultNode);
  return GetPriorNode(notEditableNode, aEditableNode, aResultNode, bNoBlockCrossing);
}


nsresult
nsEditor::GetNextNode(nsIDOMNode   *aParentNode,
                       PRInt32      aOffset,
                       PRBool       aEditableNode,
                       nsCOMPtr<nsIDOMNode> *aResultNode,
                       PRBool       bNoBlockCrossing)
{
  // just another version of GetNextNode that takes a {parent, offset}
  // instead of a node
  if (!aParentNode || !aResultNode) { return NS_ERROR_NULL_POINTER; }

  *aResultNode = nsnull;

  // if aParentNode is a text node, use it's location instead
  if (IsTextNode(aParentNode))
  {
    nsCOMPtr<nsIDOMNode> parent;
    nsEditor::GetNodeLocation(aParentNode, address_of(parent), &aOffset);
    aParentNode = parent;
    aOffset++;  // _after_ the text node
  }
  // look at the child at 'aOffset'
  nsCOMPtr<nsIDOMNode> child = GetChildAt(aParentNode, aOffset);
  if (child)
  {
    if (bNoBlockCrossing && IsBlockNode(child))
    {
      *aResultNode = child;  // return this block
      return NS_OK;
    }
    *aResultNode = GetLeftmostChild(child, bNoBlockCrossing);
    if (!*aResultNode)
    {
      *aResultNode = child;
      return NS_OK;
    }
    if (!IsDescendantOfBody(*aResultNode))
    {
      *aResultNode = nsnull;
      return NS_OK;
    }

    if (!aEditableNode || IsEditable(*aResultNode))
      return NS_OK;

    // restart the search from the non-editable node we just found
    nsCOMPtr<nsIDOMNode> notEditableNode = do_QueryInterface(*aResultNode);
    return GetNextNode(notEditableNode, aEditableNode, aResultNode, bNoBlockCrossing);
  }

  // unless there isn't one, in which case we are at the end of the node
  // and want the next one.
  if (bNoBlockCrossing && IsBlockNode(aParentNode))
  {
    // don't cross out of parent block
    return NS_OK;
  }
  return GetNextNode(aParentNode, aEditableNode, aResultNode, bNoBlockCrossing);
}


nsresult
nsEditor::GetPriorNode(nsIDOMNode  *aCurrentNode,
                       PRBool       aEditableNode,
                       nsCOMPtr<nsIDOMNode> *aResultNode,
                       PRBool       bNoBlockCrossing)
{
  nsresult result;
  if (!aCurrentNode || !aResultNode) { return NS_ERROR_NULL_POINTER; }

  *aResultNode = nsnull;  // init out-param

  if (IsRootNode(aCurrentNode))
  {
    // Don't allow traversal above the root node! This helps
    // prevent us from accidentally editing browser content
    // when the editor is in a text widget.

    return NS_OK;
  }

  nsCOMPtr<nsIDOMNode> candidate;
  result = GetPriorNodeImpl(aCurrentNode, aEditableNode, address_of(candidate), bNoBlockCrossing);
  if (NS_FAILED(result)) return result;

  if (!candidate)
  {
    // we could not find a prior node.  return null.
    return NS_OK;
  }
  else if (!aEditableNode) *aResultNode = candidate;
  else if (IsEditable(candidate)) *aResultNode = candidate;
  else
  { // restart the search from the non-editable node we just found
    nsCOMPtr<nsIDOMNode> notEditableNode = do_QueryInterface(candidate);
    return GetPriorNode(notEditableNode, aEditableNode, aResultNode, bNoBlockCrossing);
  }
  return result;
}

nsresult
nsEditor::GetPriorNodeImpl(nsIDOMNode  *aCurrentNode,
                           PRBool       aEditableNode,
                           nsCOMPtr<nsIDOMNode> *aResultNode,
                           PRBool       bNoBlockCrossing)
{
  // called only by GetPriorNode so we don't need to check params.

  // if aCurrentNode has a left sibling, return that sibling's rightmost child (or itself if it has no children)
  nsCOMPtr<nsIDOMNode> prevSibling;
  nsresult result = aCurrentNode->GetPreviousSibling(getter_AddRefs(prevSibling));
  if ((NS_SUCCEEDED(result)) && prevSibling)
  {
    if (bNoBlockCrossing && IsBlockNode(prevSibling))
    {
      // don't look inside prevsib, since it is a block
      *aResultNode = prevSibling;
      return NS_OK;
    }
    *aResultNode = GetRightmostChild(prevSibling, bNoBlockCrossing);
    if (!*aResultNode)
    {
      *aResultNode = prevSibling;
      return NS_OK;
    }
    if (!IsDescendantOfBody(*aResultNode))
    {
      *aResultNode = nsnull;
      return NS_OK;
    }
  }
  else
  {
    // otherwise, walk up the parent tree until there is a child that comes before
    // the ancestor of aCurrentNode.  Then return that node's rightmost child
    nsCOMPtr<nsIDOMNode> parent = do_QueryInterface(aCurrentNode);
    nsCOMPtr<nsIDOMNode> node, notEditableNode;
    do {
      node = parent;
      result = node->GetParentNode(getter_AddRefs(parent));
      if ((NS_SUCCEEDED(result)) && parent)
      {
        if (!IsDescendantOfBody(parent))
        {
          *aResultNode = nsnull;
          return NS_OK;
        }
        if ((bNoBlockCrossing && IsBlockNode(parent)) || IsRootNode(parent))
        {
          // we are at front of block or root, do not step out
          *aResultNode = nsnull;
          return NS_OK;
        }
        result = parent->GetPreviousSibling(getter_AddRefs(node));
        if ((NS_SUCCEEDED(result)) && node)
        {
          if (bNoBlockCrossing && IsBlockNode(node))
          {
            // prev sibling is a block, do not step into it
            *aResultNode = node;
            return NS_OK;
          }
          *aResultNode = GetRightmostChild(node, bNoBlockCrossing);
          if (!*aResultNode)  *aResultNode = node;
          return NS_OK;
        }
      }
    } while ((NS_SUCCEEDED(result)) && parent && !*aResultNode);
  }
  return result;
}

nsresult
nsEditor::GetNextNode(nsIDOMNode  *aCurrentNode,
                      PRBool       aEditableNode,
                      nsCOMPtr<nsIDOMNode> *aResultNode,
                      PRBool       bNoBlockCrossing)
{
  if (!aCurrentNode || !aResultNode) { return NS_ERROR_NULL_POINTER; }

  *aResultNode = nsnull;  // init out-param

  if (IsRootNode(aCurrentNode))
  {
    // Don't allow traversal above the root node! This helps
    // prevent us from accidentally editing browser content
    // when the editor is in a text widget.

    return NS_OK;
  }

  nsCOMPtr<nsIDOMNode> candidate;
  nsresult result = GetNextNodeImpl(aCurrentNode, aEditableNode,
                                    address_of(candidate), bNoBlockCrossing);
  if (NS_FAILED(result)) return result;

  if (!candidate)
  {
    // we could not find a next node.  return null.
    *aResultNode = nsnull;
    return NS_OK;
  }
  else if (!aEditableNode) *aResultNode = candidate;
  else if (IsEditable(candidate)) *aResultNode = candidate;
  else
  { // restart the search from the non-editable node we just found
    nsCOMPtr<nsIDOMNode> notEditableNode = do_QueryInterface(candidate);
    return GetNextNode(notEditableNode, aEditableNode, aResultNode, bNoBlockCrossing);
  }
  return result;
}


nsresult
nsEditor::GetNextNodeImpl(nsIDOMNode  *aCurrentNode,
                          PRBool       aEditableNode,
                          nsCOMPtr<nsIDOMNode> *aResultNode,
                          PRBool       bNoBlockCrossing)
{
  // called only by GetNextNode so we don't need to check params.

  // if aCurrentNode has a right sibling, return that sibling's leftmost child (or itself if it has no children)
  nsCOMPtr<nsIDOMNode> nextSibling;
  nsresult result = aCurrentNode->GetNextSibling(getter_AddRefs(nextSibling));
  if ((NS_SUCCEEDED(result)) && nextSibling)
  {
    if (bNoBlockCrossing && IsBlockNode(nextSibling))
    {
      // next sibling is a block, do not step into it
      *aResultNode = nextSibling;
      return NS_OK;
    }
    *aResultNode = GetLeftmostChild(nextSibling, bNoBlockCrossing);
    if (!*aResultNode)
    {
      *aResultNode = nextSibling;
      return NS_OK;
    }
    if (!IsDescendantOfBody(*aResultNode))
    {
      *aResultNode = nsnull;
      return NS_OK;
    }
  }
  else
  {
    // otherwise, walk up the parent tree until there is a child that comes after
    // the ancestor of aCurrentNode.  Then return that node's leftmost child
    nsCOMPtr<nsIDOMNode> parent(do_QueryInterface(aCurrentNode));
    nsCOMPtr<nsIDOMNode> node, notEditableNode;
    do {
      node = parent;
      result = node->GetParentNode(getter_AddRefs(parent));
      if ((NS_SUCCEEDED(result)) && parent)
      {
        if (!IsDescendantOfBody(parent))
        {
          *aResultNode = nsnull;
          return NS_OK;
        }
        if ((bNoBlockCrossing && IsBlockNode(parent)) || IsRootNode(parent))
        {
          // we are at end of block or root, do not step out
          *aResultNode = nsnull;
          return NS_OK;
        }
        result = parent->GetNextSibling(getter_AddRefs(node));
        if ((NS_SUCCEEDED(result)) && node)
        {
          if (bNoBlockCrossing && IsBlockNode(node))
          {
            // next sibling is a block, do not step into it
            *aResultNode = node;
            return NS_OK;
          }
          *aResultNode = GetLeftmostChild(node, bNoBlockCrossing);
          if (!*aResultNode) *aResultNode = node;
          return NS_OK;
        }
      }
    } while ((NS_SUCCEEDED(result)) && parent);
  }
  return result;
}


nsCOMPtr<nsIDOMNode>
nsEditor::GetRightmostChild(nsIDOMNode *aCurrentNode,
                            PRBool bNoBlockCrossing)
{
  if (!aCurrentNode) return nsnull;
  nsCOMPtr<nsIDOMNode> resultNode, temp=aCurrentNode;
  PRBool hasChildren;
  aCurrentNode->HasChildNodes(&hasChildren);
  while (hasChildren)
  {
    temp->GetLastTraversalChild(getter_AddRefs(resultNode));
    if (resultNode && (temp != resultNode))
    {
      if (bNoBlockCrossing && IsBlockNode(resultNode))
         return resultNode;
      resultNode->HasChildNodes(&hasChildren);
      temp = resultNode;
    }
    else
      hasChildren = PR_FALSE;
  }

  return resultNode;
}

nsCOMPtr<nsIDOMNode>
nsEditor::GetLeftmostChild(nsIDOMNode *aCurrentNode,
                           PRBool bNoBlockCrossing)
{
  if (!aCurrentNode) return nsnull;
  nsCOMPtr<nsIDOMNode> resultNode, temp=aCurrentNode;
  PRBool hasChildren;
  aCurrentNode->HasChildNodes(&hasChildren);
  while (hasChildren)
  {
    temp->GetFirstChild(getter_AddRefs(resultNode));
    if (resultNode)
    {
      if (bNoBlockCrossing && IsBlockNode(resultNode))
         return resultNode;
      resultNode->HasChildNodes(&hasChildren);
      temp = resultNode;
    }
    else
      hasChildren = PR_FALSE;
  }

  return resultNode;
}

PRBool
nsEditor::IsBlockNode(nsIDOMNode *aNode)
{
  // stub to be overridden in nsHTMLEditor.
  // screwing around with the class heirarchy here in order
  // to not duplicate the code in GetNextNode/GetPrevNode
  // across both nsEditor/nsHTMLEditor.
  return PR_FALSE;
}

PRBool
nsEditor::CanContainTag(nsIDOMNode* aParent, const nsAString &aChildTag)
{
  nsCOMPtr<nsIDOMElement> parentElement = do_QueryInterface(aParent);
  nsCOMPtr<nsIDOMNode> grandParentNode;
  nsCOMPtr<nsIDOMElement> grandParentElement;
  if (!parentElement) return PR_FALSE;
  nsAutoString parentTag;
  PRBool result;
  parentElement->GetTagName(parentTag);
  if (parentTag.EqualsLiteral("#text")) {
    // we want to guard against giving permission to put real text into an empty white space node
    parentElement->GetParentNode(getter_AddRefs(grandParentNode));
    if (grandParentNode) {
      grandParentElement = do_QueryInterface(grandParentElement);
      grandParentElement->GetTagName(parentTag);
      return TagCanContainTag(parentTag, aChildTag);
    }
    return PR_FALSE;
  }
  PRBool isMath = nsHTMLEditUtils::IsMath(aParent);
  if (isMath)
  {
    if (aChildTag.EqualsLiteral("math")) return PR_FALSE;
    if (aChildTag.EqualsLiteral("msibr")) return PR_FALSE;
    if (aChildTag.EqualsLiteral("mspace") || aChildTag.EqualsLiteral("msirule")) {
      if (parentTag.EqualsLiteral("math") ||
          parentTag.EqualsLiteral("mrow")) return PR_TRUE;
      else return PR_FALSE;
    }
    if (aChildTag.EqualsLiteral("#text")) return PR_TRUE;
     // BBM: this probably needs more cases
    return PR_TRUE;
  }
  else
  {
    if (parentTag.EqualsLiteral("msiframe")) {
      NS_NAMED_LITERAL_STRING(frameType, "frameType");
      nsAutoString type;
      GetAttributeValue(parentElement, frameType, type, &result);
      if (result && type.Length() > 0) return PR_FALSE;
    }
    return TagCanContainTag(parentTag, aChildTag);
  }
}

PRBool
nsEditor::TagCanContain(const nsAString &aParentTag, nsIDOMNode* aChild)
{
  nsAutoString childStringTag;

  if (IsTextNode(aChild))
  {
    childStringTag.AssignLiteral("#text");
  }
  else
  {
    nsCOMPtr<nsIDOMElement> childElement = do_QueryInterface(aChild);
    if (!childElement) return PR_FALSE;
    childElement->GetTagName(childStringTag);
  }
  return TagCanContainTag(aParentTag, childStringTag);
}

PRBool
nsEditor::TagCanContainTag(const nsAString &aParentTag, const nsAString &aChildTag)
{
  return PR_TRUE;
}

PRBool
nsEditor::IsRootNode(nsIDOMNode *inNode)
{
  if (!inNode)
    return PR_FALSE;

  nsIDOMElement *rootElement = GetRoot();

  nsCOMPtr<nsIDOMNode> rootNode = do_QueryInterface(rootElement);

  return inNode == rootNode;
}

PRBool
nsEditor::IsDescendantOfBody(nsIDOMNode *inNode)
{
  if (!inNode) return PR_FALSE;
  nsIDOMElement *rootElement = GetRoot();
  if (!rootElement) return PR_FALSE;
  nsCOMPtr<nsIDOMNode> root = do_QueryInterface(rootElement);

  if (inNode == root.get()) return PR_TRUE;

  nsCOMPtr<nsIDOMNode> parent, node = do_QueryInterface(inNode);

  do
  {
    node->GetParentNode(getter_AddRefs(parent));
    if (parent == root) return PR_TRUE;
    node = parent;
  } while (parent);

  return PR_FALSE;
}

PRBool
nsEditor::IsContainer(nsIDOMNode *aNode)
{
  return aNode ? PR_TRUE : PR_FALSE;
}

PRBool
nsEditor::IsTextInDirtyFrameVisible(nsIDOMNode *aNode)
{
  // virtual method
  //
  // If this is a simple non-html editor,
  // the best we can do is to assume it's visible.

  return PR_TRUE;
}

PRBool
nsEditor::IsEditable(nsIDOMNode *aNode)
{
  if (!aNode) return PR_FALSE;
  nsCOMPtr<nsIPresShell> shell;
  GetPresShell(getter_AddRefs(shell));
  if (!shell)  return PR_FALSE;

  if (IsMozEditorBogusNode(aNode) || !IsModifiableNode(aNode)) return PR_FALSE;

  // see if it has a frame.  If so, we'll edit it.
  // special case for textnodes: frame must have width.
  nsCOMPtr<nsIContent> content = do_QueryInterface(aNode);
  if (content)
  {
    nsIFrame *resultFrame = shell->GetPrimaryFrameFor(content);
    if (!resultFrame)   // if it has no frame, it is not editable
      return PR_FALSE;
    NS_ASSERTION(content->IsNodeOfType(nsINode::eTEXT) ||
                 content->IsNodeOfType(nsINode::eELEMENT),
                 "frame for non element-or-text?");
    if (!content->IsNodeOfType(nsINode::eTEXT))
      return PR_TRUE;  // not a text node; has a frame
    if (resultFrame->GetStateBits() & NS_FRAME_IS_DIRTY) // we can only trust width data for undirty frames
    {
      // In the past a comment said:
      //   "assume all text nodes with dirty frames are editable"
      // Nowadays we use a virtual function, that assumes TRUE
      // in the simple editor world,
      // and uses enhanced logic to find out in the HTML world.
      return IsTextInDirtyFrameVisible(aNode);
    }
    if (resultFrame->GetSize().width > 0)
      return PR_TRUE;  // text node has width
  }
  return PR_FALSE;  // didn't pass any editability test
}

PRBool
nsEditor::IsMozEditorBogusNode(nsIDOMNode *aNode)
{
  if (!aNode)
    return PR_FALSE;

  nsCOMPtr<nsIDOMElement>element = do_QueryInterface(aNode);
  if (element)
  {
    nsAutoString val;
    (void)element->GetAttribute(kMOZEditorBogusNodeAttr, val);
    if (val.Equals(kMOZEditorBogusNodeValue)) {
      return PR_TRUE;
    }
  }

  return PR_FALSE;
}

nsresult
nsEditor::CountEditableChildren(nsIDOMNode *aNode, PRUint32 &outCount)
{
  outCount = 0;
  if (!aNode) { return NS_ERROR_NULL_POINTER; }
  nsresult res=NS_OK;
  PRBool hasChildNodes;
  aNode->HasChildNodes(&hasChildNodes);
  if (hasChildNodes)
  {
    nsCOMPtr<nsIDOMNodeList>nodeList;
    res = aNode->GetChildNodes(getter_AddRefs(nodeList));
    if (NS_SUCCEEDED(res) && nodeList)
    {
      PRUint32 i;
      PRUint32 len;
      nodeList->GetLength(&len);
      for (i=0 ; i<len; i++)
      {
        nsCOMPtr<nsIDOMNode> child;
        res = nodeList->Item((PRInt32)i, getter_AddRefs(child));
        if ((NS_SUCCEEDED(res)) && (child))
        {
          if (IsEditable(child))
          {
            outCount++;
          }
        }
      }
    }
    else if (!nodeList)
      res = NS_ERROR_NULL_POINTER;
  }
  return res;
}

//END nsEditor static utility methods


NS_IMETHODIMP nsEditor::IncrementModificationCount(PRInt32 inNumMods)
{
  PRUint32 oldModCount = mModCount;

  mModCount += inNumMods;

  if ((oldModCount == 0 && mModCount != 0)
   || (oldModCount != 0 && mModCount == 0))
    NotifyDocumentListeners(eDocumentStateChanged);
  return NS_OK;
}


NS_IMETHODIMP nsEditor::GetModificationCount(PRInt32 *outModCount)
{
  NS_ENSURE_ARG_POINTER(outModCount);
  *outModCount = mModCount;
  return NS_OK;
}


NS_IMETHODIMP nsEditor::ResetModificationCount()
{
  PRBool doNotify = (mModCount != 0);

  mModCount = 0;

  if (doNotify)
    NotifyDocumentListeners(eDocumentStateChanged);
  return NS_OK;
}

//END nsEditor Private methods



///////////////////////////////////////////////////////////////////////////
// GetTag: digs out the atom for the tag of this node
//
nsIAtom *
nsEditor::GetTag(nsIDOMNode *aNode)
{
  if (aNode == nsnull) {
    NS_ASSERTION(aNode, "null node passed to nsEditor::Tag()");
    return nsnull;
  }

  nsCOMPtr<nsIContent> content = do_QueryInterface(aNode);
  if (!content)
  {
    PRUint16 nodeType;
    aNode->GetNodeType(&nodeType);
    if (nodeType == nsIDOMNode::TEXT_NODE) return nsGkAtoms::textTagName;
    NS_ASSERTION(content, "null node passed to nsEditor::Tag()");
    return nsnull;
  }

  return content->Tag();
}


///////////////////////////////////////////////////////////////////////////
// GetTagString: digs out string for the tag of this node
//
nsresult
nsEditor::GetTagString(nsIDOMNode *aNode, nsAString& outString)
{
  if (!aNode)
  {
    NS_NOTREACHED("null node passed to nsEditor::GetTag()");
    return NS_ERROR_NULL_POINTER;
  }

  nsIAtom *atom = GetTag(aNode);
  if (!atom)
  {
    return NS_ERROR_FAILURE;
  }

  atom->ToString(outString);
  return NS_OK;
}


///////////////////////////////////////////////////////////////////////////
// NodesSameType: do these nodes have the same tag?
//
PRBool
nsEditor::NodesSameType(nsIDOMNode *aNode1, nsIDOMNode *aNode2)
{
  if (!aNode1 || !aNode2)
  {
    NS_NOTREACHED("null node passed to nsEditor::NodesSameType()");
    return PR_FALSE;
  }

  return GetTag(aNode1) == GetTag(aNode2);
}


// IsTextOrElementNode: true if node of dom type element or text
//
PRBool
nsEditor::IsTextOrElementNode(nsIDOMNode *aNode)
{
  if (!aNode)
  {
    NS_NOTREACHED("null node passed to IsTextOrElementNode()");
    return PR_FALSE;
  }

  PRUint16 nodeType;
  aNode->GetNodeType(&nodeType);
  return ((nodeType == nsIDOMNode::ELEMENT_NODE) || (nodeType == nsIDOMNode::TEXT_NODE));
}



///////////////////////////////////////////////////////////////////////////
// IsTextNode: true if node of dom type text
//
PRBool
nsEditor::IsTextNode(nsIDOMNode *aNode)
{
  if (!aNode)
  {
    NS_NOTREACHED("null node passed to IsTextNode()");
    return PR_FALSE;
  }

  PRUint16 nodeType;
  aNode->GetNodeType(&nodeType);
  return (nodeType == nsIDOMNode::TEXT_NODE);
}


///////////////////////////////////////////////////////////////////////////
// GetIndexOf: returns the position index of the node in the parent
//
PRInt32
nsEditor::GetIndexOf(nsIDOMNode *parent, nsIDOMNode *child)
{
  nsCOMPtr<nsIContent> content = do_QueryInterface(parent);
  nsCOMPtr<nsIContent> cChild = do_QueryInterface(child);
  NS_PRECONDITION(content, "null content in nsEditor::GetIndexOf");
  NS_PRECONDITION(cChild, "null content in nsEditor::GetIndexOf");

  return content->IndexOf(cChild);
}


///////////////////////////////////////////////////////////////////////////
// GetChildAt: returns the node at this position index in the parent
//
nsCOMPtr<nsIDOMNode>
nsEditor::GetChildAt(nsIDOMNode *aParent, PRInt32 aOffset)
{
  nsCOMPtr<nsIDOMNode> resultNode;

  nsCOMPtr<nsIContent> parent = do_QueryInterface(aParent);

  if (!parent)
    return resultNode;

  resultNode = do_QueryInterface(parent->GetChildAt(aOffset));

  return resultNode;
}


///////////////////////////////////////////////////////////////////////////
// GetStartNodeAndOffset: returns whatever the start parent & offset is of
//                        the first range in the selection.
nsresult
nsEditor::GetStartNodeAndOffset(nsISelection *aSelection,
                                       nsIDOMNode **outStartNode,
                                       PRInt32 *outStartOffset)
{
  NS_ENSURE_TRUE(outStartNode && outStartOffset && aSelection, NS_ERROR_NULL_POINTER);

  *outStartNode = nsnull;
  *outStartOffset = 0;
  PRInt32 rangeCount;
  nsresult result = NS_ERROR_UNEXPECTED;

  //nsISelection* selection = static_cast<Selection*>(aSelection);
  result = aSelection->GetRangeCount(&rangeCount);
  NS_ENSURE_SUCCESS(result, result);

  nsCOMPtr<nsIDOMRange> range;
  result = aSelection->GetRangeAt(0, getter_AddRefs(range));
  NS_ENSURE_SUCCESS(result, result);
  result = range->GetStartContainer(outStartNode);
  NS_ENSURE_SUCCESS(result, result);
  result = range->GetStartOffset(outStartOffset);
  NS_ENSURE_SUCCESS(result, result);

  return result;
}


///////////////////////////////////////////////////////////////////////////
// GetEndNodeAndOffset: returns whatever the end parent & offset is of
//                        the first range in the selection.
nsresult
nsEditor::GetEndNodeAndOffset(nsISelection *aSelection,
                                       nsIDOMNode **outEndNode,
                                       PRInt32 *outEndOffset)
{
  NS_ENSURE_TRUE(outEndNode && outEndOffset && aSelection, NS_ERROR_NULL_POINTER);

  *outEndNode = nsnull;
  *outEndOffset = 0;
  PRInt32 rangeCount;
  nsresult result = NS_ERROR_UNEXPECTED;

  //nsISelection* selection = static_cast<Selection*>(aSelection);
  result = aSelection->GetRangeCount(&rangeCount);

  nsCOMPtr<nsIDOMRange> range;
  result = aSelection->GetRangeAt(0, getter_AddRefs(range));
  result  = range->GetEndContainer(outEndNode);

  result = range->GetEndOffset(outEndOffset);

  return result;
}


///////////////////////////////////////////////////////////////////////////
// IsPreformatted: checks the style info for the node for the preformatted
//                 text style.
nsresult
nsEditor::IsPreformatted(nsIDOMNode *aNode, PRBool *aResult)
{
  nsCOMPtr<nsIContent> content = do_QueryInterface(aNode);

  if (!aResult || !content) return NS_ERROR_NULL_POINTER;

  nsCOMPtr<nsIPresShell> ps = do_QueryReferent(mPresShellWeak);
  if (!ps) return NS_ERROR_NOT_INITIALIZED;

  nsIFrame *frame = ps->GetPrimaryFrameFor(content);

  // NS_ASSERTION(frame, "no frame, see bug #188946");
  if (!frame)
  {
    // Consider nodes without a style context to be NOT preformatted:
    // For instance, this is true of JS tags inside the body (which show
    // up as #text nodes but have no style context).
    *aResult = PR_FALSE;
    return NS_OK;
  }

  const nsStyleText* styleText = frame->GetStyleText();

  *aResult = styleText->WhiteSpaceIsSignificant();
  return NS_OK;
}

PRBool SplitIsAtNodeStart( nsIDOMNode * aNode, PRInt32 offset ) {
  PRInt32 counter = 0;
  PRUint32 length;
  PRUint16 nodeType;
  nsCOMPtr<nsIDOMNode> child, temp;
  // nsAutoString s;
  nsCOMPtr<nsIDOM3Node> dom3tempnode;
  nsCOMPtr<nsIContent> textContent;
  aNode->GetNodeType(&nodeType);

  // Handle text node first
  if (nodeType == nsIDOMNode::TEXT_NODE) {
    return offset == 0;
  }

  // Now handle elements
  aNode->GetFirstChild( getter_AddRefs(child));
  while (child) {
    child->GetNodeType(&nodeType);
    if (nodeType == nsIDOMNode::ELEMENT_NODE)
      return  offset <= counter;
    else {
      textContent = do_QueryInterface(child);
      if (textContent && !(textContent->TextIsOnlyWhitespace())){
        return offset <= counter;
      }
    } 
    counter++;
    child ->GetNextSibling(getter_AddRefs(temp));
    child = temp;
  }
  return PR_TRUE; // we never found any elements or text nodes with content.
}

PRBool SplitIsAtNodeEnd( nsIDOMNode * aNode, PRInt32 offset ) {
  PRInt32 length;
  nsresult result;
  nsCOMPtr<nsIContent> textContent;
  nsCOMPtr<nsIDOMNode> child;
  PRUint16 nodeType;
  aNode->GetNodeType(&nodeType);

   // Handle text node first
  if (nodeType == nsIDOMNode::TEXT_NODE) {
    textContent = do_QueryInterface(aNode);
    if (textContent) {
      length = textContent->TextLength();
      return offset >=length;
    }
    else return PR_TRUE; // text node with no text length? Weird. Don't try splitting it
  }
  else if (nodeType == nsIDOMNode::ELEMENT_NODE) {
    nsCOMPtr<nsIDOMNodeList> childNodes;
    result = aNode->GetChildNodes(getter_AddRefs(childNodes));
    if ((NS_SUCCEEDED(result)) && (childNodes))
    {
      PRUint32 count, i;
      childNodes->GetLength(&count);
      if (offset >= count) return PR_TRUE;
      // we still might be at the end if the last nodes are empty text nodes.
      for (i = offset; i < count; i++) {
        childNodes->Item(i, getter_AddRefs(child));
        if (child) {
          textContent = do_QueryInterface(child);
          if (textContent && !(textContent->TextIsOnlyWhitespace()))
            return PR_FALSE;
        }
      }
      return PR_TRUE;
    }
  }
  return PR_TRUE; // if not text or element, we don't want to split.
}


///////////////////////////////////////////////////////////////////////////
  
  // FindLevelForNode takes a node in the document and the tag name of a new node
  // to be inserted in the document as a child of *startNode*, if possible.
  // If startNode cannot contain a node of type *tag*, then we check against startNode's
  // parent. We continue until we have found the first ancestor (looking up the tree)
  // which allows nodes of type *tag* as children. If we succeed, we return the ancestor
  // as *finalNode* and the offset of its child that is in the ancestor chain.
  // The computation is similar to what is in SplitNodeDeep.

nsresult
nsEditor::FindLevelForNode(const nsAString& tag, 
                            nsIDOMNode *startNode, 
                            PRInt32 startOffset,
                            nsIDOMNode **finalNode,
                            nsIDOMNode **semiFinalNode,
                            PRInt32 *finalOffset)
{
  nsIAtom * nsatom;
  nsAutoString nodeName;
  nsString bodyName = NS_LITERAL_STRING("body");
  PRBool fCanContain;
  nsCOMPtr<nsIDOMNode> temp, tempNode;
  PRInt32 tempOffset = startOffset;
  tempNode = startNode;
  *semiFinalNode = nsnull;
  nsresult res;

  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface((nsIEditor*)this);
  if (!htmlEditor){
    return NS_ERROR_FAILURE;
  }
  nsCOMPtr<msiITagListManager> tlm;
  htmlEditor->GetTagListManager(getter_AddRefs(tlm));

  res = tempNode->GetNodeName(nodeName);
  res = tlm->TagCanContainTag(nodeName, nsatom, tag, nsatom, &fCanContain);
  *finalOffset = startOffset;
  while (tempNode && !nodeName.Equals(bodyName) && !fCanContain) {
    temp = tempNode;
    GetNodeLocation(temp, address_of(tempNode), &tempOffset);
    if (tempNode) {
      res = tempNode->GetNodeName(nodeName);
    }
    else return NS_ERROR_FAILURE;
    res = tlm->TagCanContainTag(nodeName, nsatom, tag, nsatom, &fCanContain);
  }
  if (tempNode && fCanContain) {
    *finalNode = tempNode;
    *finalOffset = tempOffset;
    *semiFinalNode = temp;
    return NS_OK;
  }
  else return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
nsEditor::FloatStructureUp( nsIDOMNode *structNode,   // N in the above description
                            nsIDOMNode *initialParent,
                            PRInt32    initialOffset,
                            msiITagListManager * tlm )
{
  nsCOMPtr<nsIDOMNode> n;
  PRInt32 offset, nextOffset;
  nsCOMPtr<nsIDOMNode> child;
  nsCOMPtr<nsIDOMNode> newChild;
  nsCOMPtr<nsIDOMNode> parent;
  nsAutoString structName;
  nsAutoString childName;
  PRBool fCanContain;
  nsIAtom * nsatom = nsnull;
  nsresult  res;

  // Initialize
  if (initialParent == nsnull) return NS_ERROR_FAILURE;
  
  n = structNode;
  res = n->GetNodeName(structName);
  child = initialParent;
  offset = initialOffset;
  // Can child be a parent for n ?

  res = child->GetNodeName(childName);
  if (/*childName.EqualsLiteral("body") ||*/ childName.EqualsLiteral("html")) return NS_ERROR_FAILURE;
  res = tlm->TagCanContainTag(childName, nsatom, structName, nsatom, &fCanContain);
  if (fCanContain) {
    res = InsertNode( structNode, initialParent, initialOffset );
    res = MoveFollowingNodes(initialParent, initialOffset + 1, structNode, tlm);
    return res;
  }
  else {
    if ( /* validate node, etc*/ PR_TRUE ) {
      child -> GetParentNode(getter_AddRefs(parent));
      nextOffset = GetIndexOf(parent, child);

      // if (SplitIsAtNodeStart(child, offset)) {
      //   offset = nextOffset;
      //   MoveFollowingNodes( child, 0, n, tlm);
      // }
      // else if (SplitIsAtNodeEnd(child, offset)) {
      //   offset = nextOffset + 1;
      //   // no need to call MoveFollowingNodes
      // } else {
        res = SplitNode(child, offset, getter_AddRefs(newChild));
        MoveFollowingNodes(child, 0, n, tlm);
        offset = nextOffset + 1;
      // }
      // now we need to copy tail of child to n;
    }
    res = FloatStructureUp(n, parent, offset, tlm);
  }
}

NS_IMETHODIMP
nsEditor::MoveFollowingNodes( nsIDOMNode *parentNode, 
        PRInt32 offset, 
        nsIDOMNode *destNode, 
        msiITagListManager * tlm) 
{
  nsCOMPtr<nsIDOMNodeList> nodelist;
  nsCOMPtr<nsIDOMNode> child;
  PRUint32 count;
  PRInt32 i;
  nsresult res;
  nsAutoString childTag;
  nsAutoString destTag;
  nsIAtom * nsatom = nsnull;
  PRBool fCanContain;

  res = destNode->GetNodeName(destTag);

  res = parentNode->GetChildNodes(getter_AddRefs(nodelist));  // note: this is a live list, so it changes as we move nodes.
  // do we need to check for parentNode a text node?
  if (NS_SUCCEEDED(res) && nodelist) {
    nodelist->GetLength(&count);
  }

  while (PR_TRUE) {
    res = nodelist->Item(offset, getter_AddRefs(child));
    if (!child) break;
    res = child->GetNodeName(childTag);
    res = tlm->TagCanContainTag(destTag, nsatom, childTag, nsatom, &fCanContain);
    if (fCanContain || childTag.EqualsLiteral("#text")) {
      MoveNode( child, destNode, -1); // offset -1 means last position
    } else break;
  }
  return res;
}



///////////////////////////////////////////////////////////////////////////
//
//  This splits a node "deeply", splitting children as
//                appropriate.  The place to split is represented by
//                a dom point at {splitPointParent, splitPointOffset}.
//                That dom point must be inside aNode, which is the node to
//                split.  outOffset is set to the offset in the parent of aNode where
//                the split terminates - where you would want to insert
//                a new element, for instance, if that's why you were splitting
//                the node. The parent of aNode is unchanged by this function.
//                
nsresult
nsEditor::SplitNodeDeep(nsIDOMNode *aNode,
                        nsIDOMNode *aSplitPointParent,
                        PRInt32 aSplitPointOffset,
                        PRInt32 *outOffset,
                        PRBool  aNoEmptyContainers,
                        nsCOMPtr<nsIDOMNode> *outLeftNode,
                        nsCOMPtr<nsIDOMNode> *outRightNode)
{
  if (!aNode || !aSplitPointParent || !outOffset) return NS_ERROR_NULL_POINTER;
  nsCOMPtr<nsIDOMNode> newLeftNode, parentNode;
  PRInt32 newOffset;
  PRInt32 offset = aSplitPointOffset;
  nsresult res;
  nsAutoString nodeName;
  PRBool done = PR_FALSE;

  if (outLeftNode)  *outLeftNode  = nsnull;
  if (outRightNode) *outRightNode = nsnull;

  // BBM: does the next line do anything??
  nsCOMPtr<nsIDOMNode> nodeToSplit = do_QueryInterface(aSplitPointParent);
  // We iteratate splitting nodes until we get to aNode, which we split.
  while (nodeToSplit && !done)
  {
    // need to insert rules code call here to do things like
    // not split a list if you are after the last <li> or before the first, etc.
    // for now we just have some smarts about unneccessarily splitting
    // textnodes, which should be universal enough to put straight in
    // this nsEditor routine.
    done = (nodeToSplit == aNode); //if done is true, this is the last run through the loop
    if (aNoEmptyContainers && SplitIsAtNodeStart(nodeToSplit, offset)) {
      nodeToSplit->GetParentNode(getter_AddRefs(parentNode));
      // Check for termination??
      offset = GetIndexOf(parentNode, nodeToSplit);
    }
    else if (aNoEmptyContainers && SplitIsAtNodeEnd(nodeToSplit, offset)) {
      nodeToSplit->GetParentNode(getter_AddRefs(parentNode));
      // Check for termination??
      offset = GetIndexOf(parentNode, nodeToSplit) + 1;
    }
    else {
      nodeToSplit->GetParentNode(getter_AddRefs(parentNode));
      newOffset = GetIndexOf(parentNode, nodeToSplit) + 1;
      res = SplitNode(nodeToSplit, offset, getter_AddRefs(newLeftNode)); 
      // Check for termination??
      offset = newOffset;
    }
    if (done) {
      *outOffset = offset;
      if (outLeftNode)  *outLeftNode  = newLeftNode;
      if (outRightNode) *outRightNode = nodeToSplit;

      // the offset is the position in the current value of nodeToSplit.
    }
    nodeToSplit = parentNode;
  }

  return NS_OK;
}


///////////////////////////////////////////////////////////////////////////
// JoinNodeDeep:  this joins two like nodes "deeply", joining children as
//                appropriate.
nsresult
nsEditor::JoinNodeDeep(nsIDOMNode *aLeftNode,
                       nsIDOMNode *aRightNode,
                       nsCOMPtr<nsIDOMNode> *aOutJoinNode,
                       PRInt32 *outOffset)
{
  if (!aLeftNode || !aRightNode || !aOutJoinNode || !outOffset) return NS_ERROR_NULL_POINTER;

  // while the rightmost children and their descendants of the left node
  // match the leftmost children and their descendants of the right node
  // join them up.  Can you say that three times fast?

  nsCOMPtr<nsIDOMNode> leftNodeToJoin = do_QueryInterface(aLeftNode);
  nsCOMPtr<nsIDOMNode> rightNodeToJoin = do_QueryInterface(aRightNode);
  nsCOMPtr<nsIDOMNode> parentNode,tmp;
  nsresult res = NS_OK;

  rightNodeToJoin->GetParentNode(getter_AddRefs(parentNode));

  while (leftNodeToJoin && rightNodeToJoin && parentNode &&
          NodesSameType(leftNodeToJoin, rightNodeToJoin))
  {
    // adjust out params
    PRUint32 length;
    if (IsTextNode(leftNodeToJoin))
    {
      nsCOMPtr<nsIDOMCharacterData>nodeAsText;
      nodeAsText = do_QueryInterface(leftNodeToJoin);
      nodeAsText->GetLength(&length);
    }
    else
    {
      res = GetLengthOfDOMNode(leftNodeToJoin, length);
      if (NS_FAILED(res)) return res;
    }

    *aOutJoinNode = rightNodeToJoin;
    *outOffset = length;

    // do the join
    res = JoinNodes(leftNodeToJoin, rightNodeToJoin, parentNode);
    if (NS_FAILED(res)) return res;

    if (IsTextNode(parentNode)) // we've joined all the way down to text nodes, we're done!
      return NS_OK;

    else
    {
      // get new left and right nodes, and begin anew
      parentNode = rightNodeToJoin;
      leftNodeToJoin = GetChildAt(parentNode, length-1);
      rightNodeToJoin = GetChildAt(parentNode, length);

      // skip over non-editable nodes
      while (leftNodeToJoin && !IsEditable(leftNodeToJoin))
      {
        leftNodeToJoin->GetPreviousSibling(getter_AddRefs(tmp));
        leftNodeToJoin = tmp;
      }
      if (!leftNodeToJoin) break;

      while (rightNodeToJoin && !IsEditable(rightNodeToJoin))
      {
        rightNodeToJoin->GetNextSibling(getter_AddRefs(tmp));
        rightNodeToJoin = tmp;
      }
      if (!rightNodeToJoin) break;
    }
  }

  return res;
}

nsresult nsEditor::BeginUpdateViewBatch()
{
  NS_PRECONDITION(mUpdateCount >= 0, "bad state");


  if (0 == mUpdateCount)
  {
    // Turn off selection updates and notifications.

    nsCOMPtr<nsISelection> selection;
    GetSelection(getter_AddRefs(selection));

    if (selection)
    {
      nsCOMPtr<nsISelectionPrivate> selPrivate(do_QueryInterface(selection));
      selPrivate->StartBatchChanges();
    }

    // Turn off view updating.
    mBatch.BeginUpdateViewBatch(mViewManager);
  }

  mUpdateCount++;

  return NS_OK;
}


nsresult nsEditor::EndUpdateViewBatch()
{
  NS_PRECONDITION(mUpdateCount > 0, "bad state");

  if (mUpdateCount <= 0)
  {
    mUpdateCount = 0;
    return NS_ERROR_FAILURE;
  }

  mUpdateCount--;

  if (0 == mUpdateCount)
  {
    // Hide the caret with an StCaretHider. By the time it goes out
    // of scope and tries to show the caret, reflow and selection changed
    // notifications should've happened so the caret should have enough info
    // to draw at the correct position.

    nsCOMPtr<nsICaret> caret;
    nsCOMPtr<nsIPresShell> presShell;
    GetPresShell(getter_AddRefs(presShell));

    if (presShell)
      presShell->GetCaret(getter_AddRefs(caret));

    StCaretHider caretHider(caret);

    PRUint32 flags = 0;

    GetFlags(&flags);

    // Turn view updating back on.
    if (mViewManager)
    {
      PRUint32 updateFlag = NS_VMREFRESH_IMMEDIATE;

      // If we're doing async updates, use NS_VMREFRESH_DEFERRED here, so that
      // the reflows we caused will get processed before the invalidates.
      if (flags & nsIPlaintextEditor::eEditorUseAsyncUpdatesMask) {
        updateFlag = NS_VMREFRESH_DEFERRED;
      } else {
        // Flush out layout.  Need to do this because if we have no invalidates
        // to flush the viewmanager code won't flush our reflow here, and we
        // have selection code that does sync caret scrolling in this case.
        presShell->FlushPendingNotifications(Flush_Layout);
      }
      mBatch.EndUpdateViewBatch(updateFlag);
    }

    // Turn selection updating and notifications back on.

    nsCOMPtr<nsISelection>selection;
    GetSelection(getter_AddRefs(selection));

    if (selection) {
      nsCOMPtr<nsISelectionPrivate>selPrivate(do_QueryInterface(selection));
      selPrivate->EndBatchChanges();
    }
  }    
  return NS_OK;
}

PRBool
nsEditor::GetShouldTxnSetSelection()
{
  return mShouldTxnSetSelection;
}




NS_IMETHODIMP
nsEditor::DeleteSelectionImpl(nsIEditor::EDirection aAction)
{
  nsCOMPtr<nsISelection>selection;
  nsresult res = GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(res)) return res;
  nsRefPtr<EditAggregateTxn> txn;
  nsCOMPtr<nsIDOMNode> deleteNode;
  PRInt32 deleteCharOffset = 0, deleteCharLength = 0;
  res = CreateTxnForDeleteSelection(aAction, getter_AddRefs(txn),
                                    getter_AddRefs(deleteNode),
                                    &deleteCharOffset, &deleteCharLength);
  nsCOMPtr<nsIDOMCharacterData> deleteCharData(do_QueryInterface(deleteNode));

  if (NS_SUCCEEDED(res))
  {
    nsAutoRules beginRulesSniffing(this, kOpDeleteSelection, aAction);
    PRInt32 i;
    // Notify nsIEditActionListener::WillDelete[Selection|Text|Node]
    if (!deleteNode)
      for (i = 0; i < mActionListeners.Count(); i++)
        mActionListeners[i]->WillDeleteSelection(selection);
    else if (deleteCharData)
      for (i = 0; i < mActionListeners.Count(); i++)
        mActionListeners[i]->WillDeleteText(deleteCharData, deleteCharOffset, 1);
    else
      for (i = 0; i < mActionListeners.Count(); i++)
        mActionListeners[i]->WillDeleteNode(deleteNode);

    // Delete the specified amount
    res = DoTransaction(txn);

    // Notify nsIEditActionListener::DidDelete[Selection|Text|Node]
    if (!deleteNode)
      for (i = 0; i < mActionListeners.Count(); i++)
        mActionListeners[i]->DidDeleteSelection(selection);
    else if (deleteCharData)
      for (i = 0; i < mActionListeners.Count(); i++)
        mActionListeners[i]->DidDeleteText(deleteCharData, deleteCharOffset, 1, res);
    else
      for (i = 0; i < mActionListeners.Count(); i++)
        mActionListeners[i]->DidDeleteNode(deleteNode, res);
  }

  return res;
}

// XXX: error handling in this routine needs to be cleaned up!
NS_IMETHODIMP
nsEditor::DeleteSelectionAndCreateNode(const nsAString& aTag,
                                           nsIDOMNode ** aNewNode)
{
  nsCOMPtr<nsIDOMNode> parentSelectedNode;
  PRInt32 offsetOfNewNode;
  nsresult result = DeleteSelectionAndPrepareToCreateNode(parentSelectedNode,
                                                          offsetOfNewNode);
  if (NS_FAILED(result))
    return result;

  nsCOMPtr<nsIDOMNode> newNode;
  result = CreateNode(aTag, parentSelectedNode, offsetOfNewNode,
                      getter_AddRefs(newNode));
  // XXX: ERROR_HANDLING  check result, and make sure aNewNode is set correctly in success/failure cases
  *aNewNode = newNode;
  NS_IF_ADDREF(*aNewNode);

  // we want the selection to be just after the new node
  nsCOMPtr<nsISelection> selection;
  result = GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(result)) return result;
  if (!selection) return NS_ERROR_NULL_POINTER;
  return selection->Collapse(parentSelectedNode, offsetOfNewNode+1);
}


/* Non-interface, protected methods */

nsresult
nsEditor::GetIMEBufferLength(PRInt32* length)
{
  *length = mIMEBufferLength;
  return NS_OK;
}

void
nsEditor::SetIsIMEComposing(){
  // We set mIsIMEComposing according to mIMETextRangeList.
  nsCOMPtr<nsIPrivateTextRange> rangePtr;
  PRUint16 listlen, type;

  mIsIMEComposing = PR_FALSE;
  nsresult result = mIMETextRangeList->GetLength(&listlen);
  if (NS_FAILED(result)) return;

  for (PRUint16 i = 0; i < listlen; i++)
  {
      result = mIMETextRangeList->Item(i, getter_AddRefs(rangePtr));
      if (NS_FAILED(result)) continue;
      result = rangePtr->GetRangeType(&type);
      if (NS_FAILED(result)) continue;
      if ( type == nsIPrivateTextRange::TEXTRANGE_RAWINPUT ||
           type == nsIPrivateTextRange::TEXTRANGE_CONVERTEDTEXT ||
           type == nsIPrivateTextRange::TEXTRANGE_SELECTEDRAWTEXT ||
           type == nsIPrivateTextRange::TEXTRANGE_SELECTEDCONVERTEDTEXT )
      {
        mIsIMEComposing = PR_TRUE;
#ifdef DEBUG_IME
        printf("nsEditor::mIsIMEComposing = PR_TRUE\n");
#endif
        break;
      }
  }
  return;
}

PRBool
nsEditor::IsIMEComposing() {
  return mIsIMEComposing;
}

NS_IMETHODIMP
nsEditor::DeleteSelectionAndPrepareToCreateNode(nsCOMPtr<nsIDOMNode> &parentSelectedNode, PRInt32& offsetOfNewNode)
{
  nsresult result=NS_ERROR_NOT_INITIALIZED;
  nsCOMPtr<nsISelection> selection;
  result = GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(result)) return result;
  if (!selection) return NS_ERROR_NULL_POINTER;

  PRBool collapsed;
  result = selection->GetIsCollapsed(&collapsed);
  if (NS_SUCCEEDED(result) && !collapsed)
  {
    result = DeleteSelection(nsIEditor::eNone);
    if (NS_FAILED(result)) {
      return result;
    }
    // get the new selection
    result = GetSelection(getter_AddRefs(selection));
    if (NS_FAILED(result)) {
      return result;
    }
#ifdef NS_DEBUG
    nsCOMPtr<nsIDOMNode>testSelectedNode;
    nsresult debugResult = selection->GetAnchorNode(getter_AddRefs(testSelectedNode));
    // no selection is ok.
    // if there is a selection, it must be collapsed
    if (testSelectedNode)
    {
      PRBool testCollapsed;
      debugResult = selection->GetIsCollapsed(&testCollapsed);
      NS_ASSERTION((NS_SUCCEEDED(result)), "couldn't get a selection after deletion");
      NS_ASSERTION(testCollapsed, "selection not reset after deletion");
    }
#endif
  }
  // split the selected node
  PRInt32 offsetOfSelectedNode;
  result = selection->GetAnchorNode(getter_AddRefs(parentSelectedNode));
  if (NS_SUCCEEDED(result) && NS_SUCCEEDED(selection->GetAnchorOffset(&offsetOfSelectedNode)) && parentSelectedNode)
  {
    nsCOMPtr<nsIDOMNode> selectedNode;
    PRUint32 selectedNodeContentCount=0;
    nsCOMPtr<nsIDOMCharacterData>selectedParentNodeAsText;
    selectedParentNodeAsText = do_QueryInterface(parentSelectedNode);

    offsetOfNewNode = offsetOfSelectedNode;

    /* if the selection is a text node, split the text node if necessary
       and compute where to put the new node
    */
    if (selectedParentNodeAsText)
    {
      PRInt32 indexOfTextNodeInParent;
      selectedNode = do_QueryInterface(parentSelectedNode);
      selectedNode->GetParentNode(getter_AddRefs(parentSelectedNode));
      selectedParentNodeAsText->GetLength(&selectedNodeContentCount);
      GetChildOffset(selectedNode, parentSelectedNode, indexOfTextNodeInParent);

      if ((offsetOfSelectedNode!=0) && (((PRUint32)offsetOfSelectedNode)!=selectedNodeContentCount))
      {
        nsCOMPtr<nsIDOMNode> newSiblingNode;
        result = SplitNode(selectedNode, offsetOfSelectedNode, getter_AddRefs(newSiblingNode));
        // now get the node's offset in it's parent, and insert the new tag there
        if (NS_SUCCEEDED(result)) {
          result = GetChildOffset(selectedNode, parentSelectedNode, offsetOfNewNode);
        }
      }
      else
      { // determine where to insert the new node
        if (0==offsetOfSelectedNode) {
          offsetOfNewNode = indexOfTextNodeInParent; // insert new node as previous sibling to selection parent
        }
        else {                 // insert new node as last child
          GetChildOffset(selectedNode, parentSelectedNode, offsetOfNewNode);
          offsetOfNewNode++;    // offsets are 0-based, and we need the index of the new node
        }
      }
    }
    // Here's where the new node was inserted
  }
#ifdef DEBUG
  else {
    printf("InsertLineBreak into an empty document is not yet supported\n");
  }
#endif
  return result;
}



NS_IMETHODIMP
nsEditor::DoAfterDoTransaction(nsITransaction *aTxn)
{
  nsresult rv = NS_OK;

  PRBool  isTransientTransaction;
  rv = aTxn->GetIsTransient(&isTransientTransaction);
  if (NS_FAILED(rv))
    return rv;

  if (!isTransientTransaction)
  {
    // we need to deal here with the case where the user saved after some
    // edits, then undid one or more times. Then, the undo count is -ve,
    // but we can't let a do take it back to zero. So we flip it up to
    // a +ve number.
    PRInt32 modCount;
    GetModificationCount(&modCount);
    if (modCount < 0)
      modCount = -modCount;

    rv = IncrementModificationCount(1);    // don't count transient transactions
  }

  return rv;
}


NS_IMETHODIMP
nsEditor::DoAfterUndoTransaction()
{
  nsresult rv = NS_OK;

  rv = IncrementModificationCount(-1);    // all undoable transactions are non-transient

  return rv;
}

NS_IMETHODIMP
nsEditor::DoAfterRedoTransaction()
{
  return IncrementModificationCount(1);    // all redoable transactions are non-transient
}

NS_IMETHODIMP
nsEditor::CreateTxnForSetAttribute(nsIDOMElement *aElement,
                                   const nsAString& aAttribute,
                                   const nsAString& aValue,
                                   ChangeAttributeTxn ** aTxn)
{
  nsresult result = NS_ERROR_NULL_POINTER;
  if (nsnull != aElement)
  {
    result = TransactionFactory::GetNewTransaction(ChangeAttributeTxn::GetCID(), (EditTxn **)aTxn);
    if (NS_SUCCEEDED(result))  {
      result = (*aTxn)->Init(this, aElement, aAttribute, aValue, PR_FALSE);
    }
  }
  return result;
}


NS_IMETHODIMP
nsEditor::CreateTxnForRemoveAttribute(nsIDOMElement *aElement,
                                      const nsAString& aAttribute,
                                      ChangeAttributeTxn ** aTxn)
{
  nsresult result = NS_ERROR_NULL_POINTER;
  if (nsnull != aElement)
  {
    result = TransactionFactory::GetNewTransaction(ChangeAttributeTxn::GetCID(), (EditTxn **)aTxn);
    if (NS_SUCCEEDED(result))
    {
      nsAutoString value;
      result = (*aTxn)->Init(this, aElement, aAttribute, value, PR_TRUE);
    }
  }
  return result;
}


NS_IMETHODIMP nsEditor::CreateTxnForCreateElement(const nsAString& aTag,
                                                  nsIDOMNode     *aParent,
                                                  PRInt32         aPosition,
                                                  CreateElementTxn ** aTxn)
{
  nsresult result = NS_ERROR_NULL_POINTER;
  if (nsnull != aParent)
  {
    result = TransactionFactory::GetNewTransaction(CreateElementTxn::GetCID(), (EditTxn **)aTxn);
    if (NS_SUCCEEDED(result))  {
      result = (*aTxn)->Init(this, aTag, aParent, aPosition);
    }
  }
  return result;
}


NS_IMETHODIMP nsEditor::CreateTxnForInsertElement(nsIDOMNode * aNode,
                                                  nsIDOMNode * aParent,
                                                  PRInt32      aPosition,
                                                  InsertElementTxn ** aTxn)
{
  nsresult result =  NS_ERROR_NULL_POINTER;
  if (aNode && aParent && aTxn)
  {
    result = TransactionFactory::GetNewTransaction(InsertElementTxn::GetCID(), (EditTxn **)aTxn);
    if (NS_SUCCEEDED(result)) {
      // BBM: Temporary stuff
      // NS_IF_ADDREF(aNode);
      // NS_IF_ADDREF(aParent);
      result = (*aTxn)->Init(aNode, aParent, aPosition, this);
    }
  }
  return result;
}
NS_IMETHODIMP nsEditor::CreateTxnForReplaceElement(nsIDOMNode * aNewChild,
                                                   nsIDOMNode * aOldChild,
                                                   nsIDOMNode * aParent,
                                                   PRBool deepRangeUpdate,
                                                   ReplaceElementTxn ** aTxn)
{
  nsresult result = NS_ERROR_NULL_POINTER;
  if (aParent && aNewChild && aOldChild && aTxn)
  {
    result = TransactionFactory::GetNewTransaction(ReplaceElementTxn::GetCID(), (EditTxn **)aTxn);
    if (NS_SUCCEEDED(result))
      result = (*aTxn)->Init(aNewChild, aOldChild, aParent, this, deepRangeUpdate, &mRangeUpdater);
  }
  return result;
}

NS_IMETHODIMP nsEditor::CreateTxnForSaveSelection(nsISelection * selection,
                                               PlaceholderTxn ** aTxn)
{
  nsresult res = NS_ERROR_NULL_POINTER;
  if (selection && aTxn)
  {
    res = TransactionFactory::GetNewTransaction(PlaceholderTxn::GetCID(), (EditTxn **)aTxn);
    if (NS_SUCCEEDED(res) && *aTxn)
    {
      nsSelectionState * selState = new nsSelectionState();
      if (selState)
      {
        res = selState->SaveSelection(selection);
        if (NS_SUCCEEDED(res))
          res = (*aTxn)->Init(nsnull, selState, this);
      }
      else
        res = NS_ERROR_OUT_OF_MEMORY;
    }
  }
  return res;
}

NS_IMETHODIMP nsEditor::CreateTxnForDeleteElement(nsIDOMNode * aElement,
                                             DeleteElementTxn ** aTxn)
{
  nsresult result = NS_ERROR_NULL_POINTER;
  if (nsnull != aElement)
  {
    result = TransactionFactory::GetNewTransaction(DeleteElementTxn::GetCID(), (EditTxn **)aTxn);
    if (NS_SUCCEEDED(result)) {
      result = (*aTxn)->Init(this, aElement, &mRangeUpdater);
    }
  }
  return result;
}

NS_IMETHODIMP
nsEditor::CreateTxnForIMEText(const nsAString& aStringToInsert,
                              IMETextTxn ** aTxn)
{
  NS_ASSERTION(aTxn, "illegal value- null ptr- aTxn");
  if(!aTxn) return NS_ERROR_NULL_POINTER;

  nsresult  result;

  result = TransactionFactory::GetNewTransaction(IMETextTxn::GetCID(), (EditTxn **)aTxn);
  if (nsnull!=*aTxn) {
    result = (*aTxn)->Init(mIMETextNode,mIMETextOffset,mIMEBufferLength,mIMETextRangeList,aStringToInsert,mSelConWeak);
  }
  else {
    result = NS_ERROR_OUT_OF_MEMORY;
  }
  return result;
}


NS_IMETHODIMP
nsEditor::CreateTxnForAddStyleSheet(nsICSSStyleSheet* aSheet, AddStyleSheetTxn* *aTxn)
{
  nsresult rv = TransactionFactory::GetNewTransaction(AddStyleSheetTxn::GetCID(), (EditTxn **)aTxn);
  if (NS_FAILED(rv))
    return rv;

  if (! *aTxn)
    return NS_ERROR_OUT_OF_MEMORY;

  return (*aTxn)->Init(this, aSheet);
}



NS_IMETHODIMP
nsEditor::CreateTxnForRemoveStyleSheet(nsICSSStyleSheet* aSheet, RemoveStyleSheetTxn* *aTxn)
{
  nsresult rv = TransactionFactory::GetNewTransaction(RemoveStyleSheetTxn::GetCID(), (EditTxn **)aTxn);
  if (NS_FAILED(rv))
    return rv;

  if (! *aTxn)
    return NS_ERROR_OUT_OF_MEMORY;

  return (*aTxn)->Init(this, aSheet);
}


NS_IMETHODIMP
nsEditor::CreateTxnForDeleteSelection(nsIEditor::EDirection aAction,
                                      EditAggregateTxn ** aTxn,
                                      nsIDOMNode ** aNode,
                                      PRInt32 *aOffset,
                                      PRInt32 *aLength)
{
  if (!aTxn)
    return NS_ERROR_NULL_POINTER;
  *aTxn = nsnull;

  nsCOMPtr<nsISelectionController> selCon = do_QueryReferent(mSelConWeak);
  if (!selCon) return NS_ERROR_NOT_INITIALIZED;
  nsCOMPtr<nsISelection> selection;
  nsresult result = selCon->GetSelection(nsISelectionController::SELECTION_NORMAL,
                                         getter_AddRefs(selection));
  if ((NS_SUCCEEDED(result)) && selection)
  {
    // Check whether the selection is collapsed and we should do nothing:
    PRBool isCollapsed;
    result = (selection->GetIsCollapsed(&isCollapsed));
    if (NS_SUCCEEDED(result) && isCollapsed && aAction == eNone)
      return NS_OK;

    // allocate the out-param transaction
    result = TransactionFactory::GetNewTransaction(EditAggregateTxn::GetCID(), (EditTxn **)aTxn);
    if (NS_FAILED(result)) {
      return result;
    }

    nsCOMPtr<nsISelectionPrivate>selPrivate(do_QueryInterface(selection));
    nsCOMPtr<nsIEnumerator> enumerator;
    result = selPrivate->GetEnumerator(getter_AddRefs(enumerator));
    if (NS_SUCCEEDED(result) && enumerator)
    {
      for (enumerator->First(); NS_OK!=enumerator->IsDone(); enumerator->Next())
      {
        nsCOMPtr<nsISupports> currentItem;
        result = enumerator->CurrentItem(getter_AddRefs(currentItem));
        if ((NS_SUCCEEDED(result)) && (currentItem))
        {
          nsCOMPtr<nsIDOMRange> range( do_QueryInterface(currentItem) );
          range->GetCollapsed(&isCollapsed);
          if (!isCollapsed)
          {
            nsRefPtr<EditTxn> editTxn;
            result =
              TransactionFactory::GetNewTransaction(DeleteRangeTxn::GetCID(),
                                                    getter_AddRefs(editTxn));
            nsRefPtr<DeleteRangeTxn> txn =
              static_cast<DeleteRangeTxn*>(editTxn.get());
            if (NS_SUCCEEDED(result) && txn)
            {
              txn->Init(this, range, &mRangeUpdater);
              (*aTxn)->AppendChild(txn);
            }
            else
              result = NS_ERROR_OUT_OF_MEMORY;
          }
          // Same with range as with selection; if it is collapsed and action
          // is eNone, do nothing.
          else if (aAction != eNone)
          { // we have an insertion point.  delete the thing in front of it or behind it, depending on aAction
            result = CreateTxnForDeleteInsertionPoint(range, aAction, *aTxn, aNode, aOffset, aLength);
          }
        }
      }
    }
  }

  // if we didn't build the transaction correctly, destroy the out-param transaction so we don't leak it.
  if (NS_FAILED(result))
  {
    NS_IF_RELEASE(*aTxn);
  }

  return result;
}

nsresult
nsEditor::CreateTxnForDeleteCharacter(nsIDOMCharacterData  *aData,
                                      PRUint32              aOffset,
                                      nsIEditor::EDirection aDirection,
                                      DeleteTextTxn       **aTxn)
{
  NS_ASSERTION(aDirection == eNext || aDirection == ePrevious,
               "invalid direction");
  nsAutoString data;
  aData->GetData(data);
  PRUint32 segOffset, segLength = 1;
  if (aDirection == eNext) {
    segOffset = aOffset;
    if (NS_IS_HIGH_SURROGATE(data[segOffset]) &&
        segOffset + 1 < data.Length() &&
        NS_IS_LOW_SURROGATE(data[segOffset+1])) {
      // delete both halves of the surrogate pair
      ++segLength;
    }
  } else {
    segOffset = aOffset - 1;
    if (NS_IS_LOW_SURROGATE(data[segOffset]) &&
        segOffset > 0 &&
        NS_IS_HIGH_SURROGATE(data[segOffset-1])) {
      ++segLength;
      --segOffset;
    }
  }
  return CreateTxnForDeleteText(aData, segOffset, segLength, aTxn);
}

//XXX: currently, this doesn't handle edge conditions because GetNext/GetPrior are not implemented
NS_IMETHODIMP
nsEditor::CreateTxnForDeleteInsertionPoint(nsIDOMRange          *aRange,
                                           nsIEditor::EDirection aAction,
                                           EditAggregateTxn     *aTxn,
                                           nsIDOMNode          **aNode,
                                           PRInt32              *aOffset,
                                           PRInt32              *aLength)
{
  NS_ASSERTION(aAction == eNext || aAction == ePrevious, "invalid action");

  // get the node and offset of the insertion point
  nsCOMPtr<nsIDOMNode> node;
  nsresult result = aRange->GetStartContainer(getter_AddRefs(node));
  if (NS_FAILED(result))
    return result;

  PRInt32 offset;
  result = aRange->GetStartOffset(&offset);
  if (NS_FAILED(result))
    return result;

  // determine if the insertion point is at the beginning, middle, or end of the node
  nsCOMPtr<nsIDOMCharacterData> nodeAsText = do_QueryInterface(node);

  PRUint32 count=0;

  if (nodeAsText)
    nodeAsText->GetLength(&count);
  else
  {
    // get the child list and count
    nsCOMPtr<nsIDOMNodeList>childList;
    result = node->GetChildNodes(getter_AddRefs(childList));
    if ((NS_SUCCEEDED(result)) && childList)
      childList->GetLength(&count);
  }

  PRBool isFirst = (0 == offset);
  PRBool isLast  = (count == (PRUint32)offset);

  // XXX: if isFirst && isLast, then we'll need to delete the node
  //      as well as the 1 child

  // build a transaction for deleting the appropriate data
  // XXX: this has to come from rule section
  if ((ePrevious==aAction) && (PR_TRUE==isFirst))
  { // we're backspacing from the beginning of the node.  Delete the first thing to our left
    nsCOMPtr<nsIDOMNode> priorNode;
    result = GetPriorNode(node, PR_TRUE, address_of(priorNode));
    if ((NS_SUCCEEDED(result)) && priorNode)
    { // there is a priorNode, so delete it's last child (if text content, delete the last char.)
      // if it has no children, delete it
      nsCOMPtr<nsIDOMCharacterData> priorNodeAsText = do_QueryInterface(priorNode);
      if (priorNodeAsText)
      {
        PRUint32 length=0;
        priorNodeAsText->GetLength(&length);
        if (0<length)
        {
          DeleteTextTxn *txn;
          result = CreateTxnForDeleteCharacter(priorNodeAsText, length,
                                               ePrevious, &txn);
          if (NS_SUCCEEDED(result)) {
            aTxn->AppendChild(txn);
            NS_ADDREF(*aNode = priorNode);
            *aOffset = txn->GetOffset();
            *aLength = txn->GetNumCharsToDelete();
            NS_RELEASE(txn);
          }
        }
        else
        { // XXX: can you have an empty text node?  If so, what do you do?
          //printf("ERROR: found a text node with 0 characters\n");
          result = NS_ERROR_UNEXPECTED;
        }
      }
      else
      { // priorNode is not text, so tell it's parent to delete it
        DeleteElementTxn *txn;
        result = CreateTxnForDeleteElement(priorNode, &txn);
        if (NS_SUCCEEDED(result)) {
          aTxn->AppendChild(txn);
          NS_RELEASE(txn);
          NS_ADDREF(*aNode = priorNode);
        }
      }
    }
  }
  else if ((nsIEditor::eNext==aAction) && (PR_TRUE==isLast))
  { // we're deleting from the end of the node.  Delete the first thing to our right
    nsCOMPtr<nsIDOMNode> nextNode;
    result = GetNextNode(node, PR_TRUE, address_of(nextNode));
    if ((NS_SUCCEEDED(result)) && nextNode)
    { // there is a nextNode, so delete it's first child (if text content, delete the first char.)
      // if it has no children, delete it
      nsCOMPtr<nsIDOMCharacterData> nextNodeAsText = do_QueryInterface(nextNode);
      if (nextNodeAsText)
      {
        PRUint32 length=0;
        nextNodeAsText->GetLength(&length);
        if (0<length)
        {
          DeleteTextTxn *txn;
          result = CreateTxnForDeleteCharacter(nextNodeAsText, 0, eNext, &txn);
          if (NS_SUCCEEDED(result)) {
            aTxn->AppendChild(txn);
            NS_ADDREF(*aNode = nextNode);
            *aOffset = txn->GetOffset();
            *aLength = txn->GetNumCharsToDelete();
            NS_RELEASE(txn);
          }
        }
        else
        { // XXX: can you have an empty text node?  If so, what do you do?
          //printf("ERROR: found a text node with 0 characters\n");
          result = NS_ERROR_UNEXPECTED;
        }
      }
      else
      { // nextNode is not text, so tell it's parent to delete it
        DeleteElementTxn *txn;
        result = CreateTxnForDeleteElement(nextNode, &txn);
        if (NS_SUCCEEDED(result)) {
          aTxn->AppendChild(txn);
          NS_RELEASE(txn);
          NS_ADDREF(*aNode = nextNode);
        }
      }
    }
  }
  else
  {
    if (nodeAsText)
    { // we have text, so delete a char at the proper offset
      nsRefPtr<DeleteTextTxn> txn;
      result = CreateTxnForDeleteCharacter(nodeAsText, offset, aAction,
                                           getter_AddRefs(txn));
      if (NS_SUCCEEDED(result)) {
        aTxn->AppendChild(txn);
        NS_ADDREF(*aNode = node);
        *aOffset = txn->GetOffset();
        *aLength = txn->GetNumCharsToDelete();
      }
    }
    else
    { // we're either deleting a node or some text, need to dig into the next/prev node to find out
      nsCOMPtr<nsIDOMNode> selectedNode;
      if (ePrevious==aAction)
      {
        result = GetPriorNode(node, offset, PR_TRUE, address_of(selectedNode));
      }
      else if (eNext==aAction)
      {
        result = GetNextNode(node, offset, PR_TRUE, address_of(selectedNode));
      }
      if (NS_FAILED(result)) { return result; }
      if (selectedNode)
      {
        nsCOMPtr<nsIDOMCharacterData> selectedNodeAsText =
                                             do_QueryInterface(selectedNode);
        if (selectedNodeAsText)
        { // we are deleting from a text node, so do a text deletion
          PRUint32 position = 0;    // default for forward delete
          if (ePrevious==aAction)
          {
            selectedNodeAsText->GetLength(&position);
          }
          nsRefPtr<DeleteTextTxn> delTextTxn;
          result = CreateTxnForDeleteCharacter(selectedNodeAsText, position,
                                               aAction,
                                               getter_AddRefs(delTextTxn));
          if (NS_FAILED(result))  { return result; }
          if (!delTextTxn) { return NS_ERROR_NULL_POINTER; }
          aTxn->AppendChild(delTextTxn);
          NS_ADDREF(*aNode = selectedNode);
          *aOffset = delTextTxn->GetOffset();
          *aLength = delTextTxn->GetNumCharsToDelete();
        }
        else
        {
          nsRefPtr<DeleteElementTxn> delElementTxn;
          result = CreateTxnForDeleteElement(selectedNode,
                                             getter_AddRefs(delElementTxn));
          if (NS_FAILED(result))  { return result; }
          if (!delElementTxn) { return NS_ERROR_NULL_POINTER; }
          aTxn->AppendChild(delElementTxn);
          NS_ADDREF(*aNode = selectedNode);
        }
      }
    }
  }
  return result;
}

nsresult
nsEditor::CreateRange(nsIDOMNode *aStartParent, PRInt32 aStartOffset,
                      nsIDOMNode *aEndParent, PRInt32 aEndOffset,
                      nsIDOMRange **aRange)
{
  nsresult result;
  result = CallCreateInstance("@mozilla.org/content/range;1", aRange);
  if (NS_FAILED(result))
    return result;

  if (!*aRange)
    return NS_ERROR_NULL_POINTER;

  result = (*aRange)->SetStart(aStartParent, aStartOffset);

  if (NS_SUCCEEDED(result))
    result = (*aRange)->SetEnd(aEndParent, aEndOffset);

  if (NS_FAILED(result))
  {
    NS_RELEASE((*aRange));
    *aRange = 0;
  }
  return result;
}

nsresult
nsEditor::AppendNodeToSelectionAsRange(nsIDOMNode *aNode)
{
  if (!aNode) return NS_ERROR_NULL_POINTER;
  nsCOMPtr<nsISelection> selection;
  nsresult res = GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(res)) return res;
  if(!selection) return NS_ERROR_FAILURE;

  nsCOMPtr<nsIDOMNode> parentNode;
  res = aNode->GetParentNode(getter_AddRefs(parentNode));
  if (NS_FAILED(res)) return res;
  if (!parentNode) return NS_ERROR_NULL_POINTER;

  PRInt32 offset;
  res = GetChildOffset(aNode, parentNode, offset);
  if (NS_FAILED(res)) return res;

  nsCOMPtr<nsIDOMRange> range;
  res = CreateRange(parentNode, offset, parentNode, offset+1, getter_AddRefs(range));
  if (NS_FAILED(res)) return res;
  if (!range) return NS_ERROR_NULL_POINTER;

  return selection->AddRange(range);
}

nsresult nsEditor::ClearSelection()
{
  nsCOMPtr<nsISelection> selection;
  nsresult res = nsEditor::GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(res)) return res;
  if (!selection) return NS_ERROR_FAILURE;
  return selection->RemoveAllRanges();
}

nsresult
nsEditor::CreateHTMLContent(const nsAString& aTag, nsIContent** aContent)
{
  nsCOMPtr<nsIDOMDocument> tempDoc;
  GetDocument(getter_AddRefs(tempDoc));
  if (!tempDoc)
    return NS_ERROR_FAILURE;
  nsCOMPtr<nsIDocument> doc = do_QueryInterface(tempDoc);
  if (!doc)
    return NS_ERROR_FAILURE;

  // XXX Wallpaper over editor bug (editor tries to create elements with an
  //     empty nodename).
  if (aTag.IsEmpty()) {
    NS_ERROR("Don't pass an empty tag to nsEditor::CreateHTMLContent, "
             "check caller.");
    return NS_ERROR_FAILURE;
  }

  nsCOMPtr<nsIAtom> tag = do_GetAtom(aTag);
  if (!tag)
    return NS_ERROR_OUT_OF_MEMORY;

  nsCOMPtr<nsIHTMLDocument> htmlDoc = do_QueryInterface(tempDoc);
  if (htmlDoc) {
      return doc->CreateElem(tag, nsnull, doc->GetDefaultNamespaceID(),
                             PR_TRUE, aContent);
  }

  return doc->CreateElem(tag, nsnull, kNameSpaceID_XHTML, PR_FALSE, aContent);
}

nsresult
nsEditor::CreateContentNS(const nsAString& aQualifiedTag, nsIAtom * atomNS, nsIDOMElement** aElement)
{
  nsCOMPtr<nsIDOMDocument> tempDoc;
  GetDocument(getter_AddRefs(tempDoc));

//  nsCOMPtr<nsIDocument> doc = do_QueryInterface(tempDoc);
//  if (!doc)
//    return NS_ERROR_FAILURE;

  // XXX Wallpaper over editor bug (editor tries to create elements with an
  //     empty nodename).
  if (aQualifiedTag.IsEmpty()) {
    NS_ERROR("Don't pass an empty tag to nsEditor::CreateContentNS, "
             "check caller.");
    return NS_ERROR_FAILURE;
  }
  nsAutoString strNS;
  if (atomNS) atomNS->ToString(strNS);
  return tempDoc->CreateElementNS(strNS, aQualifiedTag, aElement);
}

nsresult
nsEditor::SetAttributeOrEquivalent(nsIDOMElement * aElement,
                                   const nsAString & aAttribute,
                                   const nsAString & aValue,
                                   PRBool aSuppressTransaction)
{
  return SetAttribute(aElement, aAttribute, aValue);
}

nsresult
nsEditor::RemoveAttributeOrEquivalent(nsIDOMElement * aElement,
                                      const nsAString & aAttribute,
                                      PRBool aSuppressTransaction)
{
  return RemoveAttribute(aElement, aAttribute);
}

nsresult
nsEditor::HandleInlineSpellCheck(PRInt32 action,
                                   nsISelection *aSelection,
                                   nsIDOMNode *previousSelectedNode,
                                   PRInt32 previousSelectedOffset,
                                   nsIDOMNode *aStartNode,
                                   PRInt32 aStartOffset,
                                   nsIDOMNode *aEndNode,
                                   PRInt32 aEndOffset)
{
  return mInlineSpellChecker ? mInlineSpellChecker->SpellCheckAfterEditorChange(action,
                                                        aSelection,
                                                        previousSelectedNode,
                                                        previousSelectedOffset,
                                                        aStartNode,
                                                        aStartOffset,
                                                        aEndNode,
                                                        aEndOffset) : NS_OK;
}

already_AddRefed<nsPIDOMEventTarget>
nsEditor::GetPIDOMEventTarget()
{
  nsPIDOMEventTarget* piTarget = mEventTarget;
  if (piTarget)
  {
    NS_ADDREF(piTarget);
    return piTarget;
  }

  nsIDOMElement *rootElement = GetRoot();

  // Now hack to make sure we are not anonymous content.
  // If we are grab the parent of root element for our observer.

  nsCOMPtr<nsIContent> content = do_QueryInterface(rootElement);

  if (content && content->IsNativeAnonymous())
  {
    mEventTarget = do_QueryInterface(content->GetParent());
    piTarget = mEventTarget;
    NS_IF_ADDREF(piTarget);
  }
  else
  {
    // Don't use getDocument here, because we have no way of knowing
    // if Init() was ever called.  So we need to get the document
    // ourselves, if it exists.
    if (mDocWeak)
    {
      CallQueryReferent(mDocWeak.get(), &piTarget);
    }
    else
    {
      NS_ERROR("not initialized yet");
    }
  }

  return piTarget;
}

nsIDOMElement *
nsEditor::GetRoot()
{
  if (!mRootElement)
  {
    nsCOMPtr<nsIDOMElement> root;

    // Let GetRootElement() do the work
    GetRootElement(getter_AddRefs(root));
  }

  return mRootElement;
}

NS_IMETHODIMP
nsEditor::SwitchTextDirection()
{
  // Get the current root direction from its frame
  nsIDOMElement *rootElement = GetRoot();

  nsresult rv;
  nsCOMPtr<nsIContent> content = do_QueryInterface(rootElement, &rv);
  if (NS_FAILED(rv))
    return rv;

  nsCOMPtr<nsIPresShell> presShell;
  rv = GetPresShell(getter_AddRefs(presShell));
  if (NS_FAILED(rv))
    return rv;

  nsIFrame *frame = presShell->GetPrimaryFrameFor(content);
  if (!frame)
    return NS_ERROR_FAILURE;

  // Apply the opposite direction
  if (frame->GetStyleVisibility()->mDirection == NS_STYLE_DIRECTION_RTL)
    rv = rootElement->SetAttribute(NS_LITERAL_STRING("dir"), NS_LITERAL_STRING("ltr"));
  else
    rv = rootElement->SetAttribute(NS_LITERAL_STRING("dir"), NS_LITERAL_STRING("rtl"));

  return rv;
}

NS_IMETHODIMP
nsEditor::SetTopXULWindow(nsIDOMWindow *window)
{
    m_window = window;
    return NS_OK;
}


PRBool
nsEditor::IsModifiableNode(nsIDOMNode *aNode)
{
  return PR_TRUE;
}

NS_IMETHODIMP
nsEditor::SimpleDeleteNode( nsIDOMNode *aNode)
{
  nsresult res;
  nsRefPtr<DeleteElementTxn> txn;
  res = CreateTxnForDeleteElement(aNode, getter_AddRefs(txn));
  if (NS_SUCCEEDED(res))  {
    res = DoTransaction(txn);
  }
  return res;
}

  /* readonly attribute boolean isCtrlDown; */
NS_IMETHODIMP
nsEditor::GetIsCtrlDown(PRBool *aIsCtrlDown) {
  *aIsCtrlDown = ctrlDown;
  return NS_OK;
}

  /* readonly attribute boolean isAltDown; */
NS_IMETHODIMP
nsEditor::GetIsAltDown(PRBool *aIsAltDown) {
  *aIsAltDown = altDown;
  return NS_OK;
}

  /* readonly attribute boolean isMetaDown; */
NS_IMETHODIMP
nsEditor::GetIsMetaDown(PRBool *aIsMetaDown) {
  *aIsMetaDown = metaDown;
  return NS_OK;
}

  /* readonly attribute boolean isShiftDown; */
NS_IMETHODIMP
nsEditor::GetIsShiftDown(PRBool *aIsShiftDown) {
  *aIsShiftDown = shiftDown;
  return NS_OK;
}

NS_IMETHODIMP
nsEditor::SetIsCtrlDown(PRBool aIsCtrlDown) {
  ctrlDown = aIsCtrlDown;
  return NS_OK;
}

  /* readonly attribute boolean isAltDown; */
NS_IMETHODIMP
nsEditor::SetIsAltDown(PRBool aIsAltDown) {
  altDown = aIsAltDown;
  return NS_OK;
}

  /* readonly attribute boolean isMetaDown; */
NS_IMETHODIMP
nsEditor::SetIsMetaDown(PRBool aIsMetaDown) {
  metaDown = aIsMetaDown;
  return NS_OK;
}

  /* readonly attribute boolean isShiftDown; */
NS_IMETHODIMP
nsEditor::SetIsShiftDown(PRBool aIsShiftDown) {
  shiftDown = aIsShiftDown;
  return NS_OK;
}


