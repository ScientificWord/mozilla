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
 *   Daniel Glazman <glazman@netscape.com>
 *   Mats Palmgren <mats.palmgren@bredband.net>
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


#include "nsPlaintextEditor.h"
#include "nsICaret.h"
#include "nsTextEditUtils.h"
#include "nsTextEditRules.h"
#include "nsEditorEventListeners.h"
#include "nsIEditActionListener.h"
#include "nsIDOMNodeList.h"
#include "nsIDOMDocument.h"
#include "nsIDocument.h"
#include "nsIDOMEventTarget.h" 
#include "nsIDOM3EventTarget.h" 
#include "nsIDOMKeyEvent.h"
#include "nsIDOMMouseListener.h"
#include "nsISelection.h"
#include "nsISelectionPrivate.h"
#include "nsISelectionController.h"
#include "nsGUIEvent.h"
#include "nsIDOMEventGroup.h"
#include "nsCRT.h"

#include "nsIEnumerator.h"
#include "nsIContent.h"
#include "nsIContentIterator.h"
#include "nsIDOMRange.h"
#include "nsISupportsArray.h"
#include "nsIComponentManager.h"
#include "nsIServiceManager.h"
#include "nsIDocumentEncoder.h"
#include "nsIPresShell.h"
#include "nsISupportsPrimitives.h"
#include "nsReadableUtils.h"

// Misc
#include "nsEditorUtils.h"  // nsAutoEditBatch, nsAutoRules
#include "nsIPrefBranch.h"
#include "nsIPrefService.h"
#include "nsUnicharUtils.h"
#include "nsContentCID.h"
#include "nsAOLCiter.h"
#include "nsInternetCiter.h"
#include "nsEventDispatcher.h"

// Drag & Drop, Clipboard
#include "nsIClipboard.h"
#include "nsITransferable.h"
#include "nsCopySupport.h"

// prototype for rules creation shortcut
nsresult NS_NewTextEditRules(nsIEditRules** aInstancePtrResult);

nsPlaintextEditor::nsPlaintextEditor()
: nsEditor()
, mIgnoreSpuriousDragEvent(PR_FALSE)
, mRules(nsnull)
, mWrapToWindow(PR_FALSE)
, mWrapColumn(0)
, mMaxTextLength(-1)
, mInitTriggerCounter(0)
, mNewlineHandling(nsIPlaintextEditor::eNewlinesPasteToFirst)
#ifdef XP_WIN
, mCaretStyle(1)
#else
, mCaretStyle(0)
#endif
{
} 

nsPlaintextEditor::~nsPlaintextEditor()
{
  // remove the rules as an action listener.  Else we get a bad ownership loop later on.
  // it's ok if the rules aren't a listener; we ignore the error.
  nsCOMPtr<nsIEditActionListener> mListener = do_QueryInterface(mRules);
  RemoveEditActionListener(mListener);
  
  // Remove event listeners. Note that if we had an HTML editor,
  //  it installed its own instead of these
  RemoveEventListeners();
}

NS_IMPL_ADDREF_INHERITED(nsPlaintextEditor, nsEditor)
NS_IMPL_RELEASE_INHERITED(nsPlaintextEditor, nsEditor)

NS_INTERFACE_MAP_BEGIN(nsPlaintextEditor)
  NS_INTERFACE_MAP_ENTRY(nsIPlaintextEditor)
  NS_INTERFACE_MAP_ENTRY(nsIEditorMailSupport)
NS_INTERFACE_MAP_END_INHERITING(nsEditor)


NS_IMETHODIMP nsPlaintextEditor::Init(nsIDOMDocument *aDoc, 
                                 nsIPresShell   *aPresShell, nsIContent *aRoot, nsISelectionController *aSelCon, PRUint32 aFlags)
{
  NS_PRECONDITION(aDoc && aPresShell, "bad arg");
  if (!aDoc || !aPresShell)
    return NS_ERROR_NULL_POINTER;
  
  nsresult res = NS_OK, rulesRes = NS_OK;
  
  if (1)
  {
    // block to scope nsAutoEditInitRulesTrigger
    nsAutoEditInitRulesTrigger rulesTrigger(this, rulesRes);
  
    // Init the base editor
    res = nsEditor::Init(aDoc, aPresShell, aRoot, aSelCon, aFlags);
  }

  // check the "single line editor newline handling"
  // and "caret behaviour in selection" prefs
  nsCOMPtr<nsIPrefBranch> prefBranch(do_GetService(NS_PREFSERVICE_CONTRACTID));
  if (prefBranch)
  {
    prefBranch->GetIntPref("editor.singleLine.pasteNewlines",
                           &mNewlineHandling);
    prefBranch->GetIntPref("layout.selection.caret_style", &mCaretStyle);
#ifdef XP_WIN
    if (mCaretStyle == 0)
      mCaretStyle = 1;
#endif
  }

  if (NS_FAILED(rulesRes)) return rulesRes;
  return res;
}

void 
nsPlaintextEditor::BeginEditorInit()
{
  mInitTriggerCounter++;
}

nsresult 
nsPlaintextEditor::EndEditorInit()
{
  nsresult res = NS_OK;
  NS_PRECONDITION(mInitTriggerCounter > 0, "ended editor init before we began?");
  mInitTriggerCounter--;
  if (mInitTriggerCounter == 0)
  {
    res = InitRules();
    if (NS_SUCCEEDED(res)) 
      EnableUndo(PR_TRUE);
  }
  return res;
}

NS_IMETHODIMP 
nsPlaintextEditor::SetDocumentCharacterSet(const nsACString & characterSet) 
{ 
  nsresult result; 

  result = nsEditor::SetDocumentCharacterSet(characterSet); 

  // update META charset tag 
  if (NS_SUCCEEDED(result)) { 
    nsCOMPtr<nsIDOMDocument>domdoc; 
    result = GetDocument(getter_AddRefs(domdoc)); 
    if (NS_SUCCEEDED(result) && domdoc) { 
      nsCOMPtr<nsIDOMNodeList>metaList; 
      nsCOMPtr<nsIDOMElement>metaElement; 
      PRBool newMetaCharset = PR_TRUE; 

      // get a list of META tags 
      result = domdoc->GetElementsByTagName(NS_LITERAL_STRING("meta"), getter_AddRefs(metaList)); 
      if (NS_SUCCEEDED(result) && metaList) { 
        PRUint32 listLength = 0; 
        (void) metaList->GetLength(&listLength); 

        nsCOMPtr<nsIDOMNode>metaNode; 
        for (PRUint32 i = 0; i < listLength; i++) { 
          metaList->Item(i, getter_AddRefs(metaNode)); 
          if (!metaNode) continue; 
          metaElement = do_QueryInterface(metaNode); 
          if (!metaElement) continue; 

          nsAutoString currentValue; 
          if (NS_FAILED(metaElement->GetAttribute(NS_LITERAL_STRING("http-equiv"), currentValue))) continue; 

          if (FindInReadable(NS_LITERAL_STRING("content-type"),
                             currentValue,
                             nsCaseInsensitiveStringComparator())) { 
            NS_NAMED_LITERAL_STRING(content, "content");
            if (NS_FAILED(metaElement->GetAttribute(content, currentValue))) continue; 

            NS_NAMED_LITERAL_STRING(charsetEquals, "charset=");
            nsAString::const_iterator originalStart, start, end;
            originalStart = currentValue.BeginReading(start);
            currentValue.EndReading(end);
            if (FindInReadable(charsetEquals, start, end,
                               nsCaseInsensitiveStringComparator())) {

              // set attribute to <original prefix> charset=text/html
              result = nsEditor::SetAttribute(metaElement, content,
                                              Substring(originalStart, start) +
                                              charsetEquals + NS_ConvertASCIItoUTF16(characterSet)); 
              if (NS_SUCCEEDED(result)) 
                newMetaCharset = PR_FALSE; 
              break; 
            } 
          } 
        } 
      } 

      if (newMetaCharset) { 
        nsCOMPtr<nsIDOMNodeList>headList; 
        result = domdoc->GetElementsByTagName(NS_LITERAL_STRING("head"),getter_AddRefs(headList)); 
        if (NS_SUCCEEDED(result) && headList) { 
          nsCOMPtr<nsIDOMNode>headNode; 
          headList->Item(0, getter_AddRefs(headNode)); 
          if (headNode) { 
            nsCOMPtr<nsIDOMNode>resultNode; 
            // Create a new meta charset tag 
            result = CreateNode(NS_LITERAL_STRING("meta"), headNode, 0, getter_AddRefs(resultNode)); 
            if (NS_FAILED(result)) 
              return NS_ERROR_FAILURE; 

            // Set attributes to the created element 
            if (resultNode && !characterSet.IsEmpty()) { 
              metaElement = do_QueryInterface(resultNode); 
              if (metaElement) { 
                // not undoable, undo should undo CreateNode 
                result = metaElement->SetAttribute(NS_LITERAL_STRING("http-equiv"), NS_LITERAL_STRING("Content-Type")); 
                if (NS_SUCCEEDED(result)) { 
                  // not undoable, undo should undo CreateNode 
                  result = metaElement->SetAttribute(NS_LITERAL_STRING("content"),
                                                     NS_LITERAL_STRING("text/html;charset=") + NS_ConvertASCIItoUTF16(characterSet)); 
                } 
              } 
            } 
          } 
        } 
      } 
    } 
  } 

  return result; 
} 

nsresult
nsPlaintextEditor::CreateEventListeners()
{
  nsresult rv = NS_OK;

  if (!mMouseListenerP) {
    // get a mouse listener
    rv |= NS_NewEditorMouseListener(getter_AddRefs(mMouseListenerP), this);
  }

  if (!mKeyListenerP) {
    // get a key listener
    rv |= NS_NewEditorKeyListener(getter_AddRefs(mKeyListenerP), this);
  }

  if (!mTextListenerP) {
    // get a text listener
    rv |= NS_NewEditorTextListener(getter_AddRefs(mTextListenerP), this);
  }

  if (!mCompositionListenerP) {
    // get a composition listener
    rv |=
      NS_NewEditorCompositionListener(getter_AddRefs(mCompositionListenerP),
                                      this);
  }

  nsCOMPtr<nsIPresShell> presShell = do_QueryReferent(mPresShellWeak);
  if (!mDragListenerP) {
    // get a drag listener
    rv |= NS_NewEditorDragListener(getter_AddRefs(mDragListenerP), presShell,
                                   this);
  }

  if (!mFocusListenerP) {
    // get a focus listener
    rv |= NS_NewEditorFocusListener(getter_AddRefs(mFocusListenerP),
                                    this, presShell);
  }

  return rv;
}

NS_IMETHODIMP 
nsPlaintextEditor::GetFlags(PRUint32 *aFlags)
{
  if (!mRules || !aFlags) { return NS_ERROR_NULL_POINTER; }
  return mRules->GetFlags(aFlags);
}


NS_IMETHODIMP 
nsPlaintextEditor::SetFlags(PRUint32 aFlags)
{
  if (!mRules) { return NS_ERROR_NULL_POINTER; }
  return mRules->SetFlags(aFlags);
}


NS_IMETHODIMP nsPlaintextEditor::InitRules()
{
  // instantiate the rules for this text editor
  nsresult res = NS_NewTextEditRules(getter_AddRefs(mRules));
  if (NS_FAILED(res)) return res;
  if (!mRules) return NS_ERROR_UNEXPECTED;
  return mRules->Init(this, mFlags);
}


NS_IMETHODIMP
nsPlaintextEditor::GetIsDocumentEditable(PRBool *aIsDocumentEditable)
{
  NS_ENSURE_ARG_POINTER(aIsDocumentEditable);

  nsCOMPtr<nsIDOMDocument> doc;
  GetDocument(getter_AddRefs(doc));
  *aIsDocumentEditable = doc ? IsModifiable() : PR_FALSE;

  return NS_OK;
}

PRBool nsPlaintextEditor::IsModifiable()
{
  PRUint32 flags;
  if (NS_SUCCEEDED(GetFlags(&flags)))
    return ((flags & eEditorReadonlyMask) == 0);

  return PR_FALSE;
}


#ifdef XP_MAC
#pragma mark -
#pragma mark  nsIHTMLEditor methods 
#pragma mark -
#endif

NS_IMETHODIMP nsPlaintextEditor::HandleKeyPress(nsIDOMKeyEvent* aKeyEvent)
{
  PRUint32 keyCode, character;
  PRBool   ctrlKey, altKey, metaKey;

  if (!aKeyEvent) return NS_ERROR_NULL_POINTER;

  if (NS_SUCCEEDED(aKeyEvent->GetKeyCode(&keyCode)) && 
      NS_SUCCEEDED(aKeyEvent->GetCtrlKey(&ctrlKey)) &&
      NS_SUCCEEDED(aKeyEvent->GetAltKey(&altKey)) &&
      NS_SUCCEEDED(aKeyEvent->GetMetaKey(&metaKey)))
  {
    aKeyEvent->GetCharCode(&character);
    if (keyCode == nsIDOMKeyEvent::DOM_VK_RETURN
     || keyCode == nsIDOMKeyEvent::DOM_VK_ENTER)
    {
      nsString empty;
      return TypedText(empty, eTypedBreak);
    }
    else if (keyCode == nsIDOMKeyEvent::DOM_VK_ESCAPE)
    {
      // pass escape keypresses through as empty strings: needed for ime support
      nsString empty;
      return TypedText(empty, eTypedText);
    }
    
    if (character && !altKey && !ctrlKey && !metaKey)
    {
      aKeyEvent->PreventDefault();
      nsAutoString key(character);
      return TypedText(key, eTypedText);
    }
  }
  return NS_ERROR_FAILURE;
}

/* This routine is needed to provide a bottleneck for typing for logging
   purposes.  Can't use HandleKeyPress() (above) for that since it takes
   a nsIDOMKeyEvent* parameter.  So instead we pass enough info through
   to TypedText() to determine what action to take, but without passing
   an event.
   */
NS_IMETHODIMP nsPlaintextEditor::TypedText(const nsAString& aString,
                                      PRInt32 aAction)
{
  nsAutoPlaceHolderBatch batch(this, gTypingTxnName);

  switch (aAction)
  {
    case eTypedText:
      {
        return InsertText(aString);
      }
    case eTypedBreak:
      {
        return InsertLineBreak();
      } 
  } 
  return NS_ERROR_FAILURE; 
}

NS_IMETHODIMP nsPlaintextEditor::CreateBRImpl(nsCOMPtr<nsIDOMNode> *aInOutParent, PRInt32 *aInOutOffset, nsCOMPtr<nsIDOMNode> *outBRNode, EDirection aSelect)
{
  if (!aInOutParent || !*aInOutParent || !aInOutOffset || !outBRNode) return NS_ERROR_NULL_POINTER;
  *outBRNode = nsnull;
  nsresult res;
  
  // we need to insert a br.  unfortunately, we may have to split a text node to do it.
  nsCOMPtr<nsIDOMNode> node = *aInOutParent;
  PRInt32 theOffset = *aInOutOffset;
  nsCOMPtr<nsIDOMCharacterData> nodeAsText = do_QueryInterface(node);
  NS_NAMED_LITERAL_STRING(brType, "br");
  nsCOMPtr<nsIDOMNode> brNode;
  if (nodeAsText)  
  {
    nsCOMPtr<nsIDOMNode> tmp;
    PRInt32 offset;
    PRUint32 len;
    nodeAsText->GetLength(&len);
    GetNodeLocation(node, address_of(tmp), &offset);
    if (!tmp) return NS_ERROR_FAILURE;
    if (!theOffset)
    {
      // we are already set to go
    }
    else if (theOffset == (PRInt32)len)
    {
      // update offset to point AFTER the text node
      offset++;
    }
    else
    {
      // split the text node
      res = SplitNode(node, theOffset, getter_AddRefs(tmp));
      if (NS_FAILED(res)) return res;
      res = GetNodeLocation(node, address_of(tmp), &offset);
      if (NS_FAILED(res)) return res;
    }
    // create br
    res = CreateNode(brType, tmp, offset, getter_AddRefs(brNode));
    if (NS_FAILED(res)) return res;
    *aInOutParent = tmp;
    *aInOutOffset = offset+1;
  }
  else
  {
    res = CreateNode(brType, node, theOffset, getter_AddRefs(brNode));
    if (NS_FAILED(res)) return res;
    (*aInOutOffset)++;
  }

  *outBRNode = brNode;
  if (*outBRNode && (aSelect != eNone))
  {
    nsCOMPtr<nsIDOMNode> parent;
    PRInt32 offset;
    res = GetNodeLocation(*outBRNode, address_of(parent), &offset);
    if (NS_FAILED(res)) return res;

    nsCOMPtr<nsISelection> selection;
    res = GetSelection(getter_AddRefs(selection));
    if (NS_FAILED(res)) return res;
    nsCOMPtr<nsISelectionPrivate> selPriv(do_QueryInterface(selection));
    if (aSelect == eNext)
    {
      // position selection after br
      selPriv->SetInterlinePosition(PR_TRUE);
      res = selection->Collapse(parent, offset+1);
    }
    else if (aSelect == ePrevious)
    {
      // position selection before br
      selPriv->SetInterlinePosition(PR_TRUE);
      res = selection->Collapse(parent, offset);
    }
  }
  return NS_OK;
}


NS_IMETHODIMP nsPlaintextEditor::CreateBR(nsIDOMNode *aNode, PRInt32 aOffset, nsCOMPtr<nsIDOMNode> *outBRNode, EDirection aSelect)
{
  nsCOMPtr<nsIDOMNode> parent = aNode;
  PRInt32 offset = aOffset;
  return CreateBRImpl(address_of(parent), &offset, outBRNode, aSelect);
}

NS_IMETHODIMP nsPlaintextEditor::InsertBR(nsCOMPtr<nsIDOMNode> *outBRNode)
{
  if (!outBRNode) return NS_ERROR_NULL_POINTER;
  *outBRNode = nsnull;

  // calling it text insertion to trigger moz br treatment by rules
  nsAutoRules beginRulesSniffing(this, kOpInsertText, nsIEditor::eNext);

  nsCOMPtr<nsISelection> selection;
  nsresult res = GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(res)) return res;
  PRBool bCollapsed;
  res = selection->GetIsCollapsed(&bCollapsed);
  if (NS_FAILED(res)) return res;
  if (!bCollapsed)
  {
    res = DeleteSelection(nsIEditor::eNone);
    if (NS_FAILED(res)) return res;
  }
  nsCOMPtr<nsIDOMNode> selNode;
  PRInt32 selOffset;
  res = GetStartNodeAndOffset(selection, address_of(selNode), &selOffset);
  if (NS_FAILED(res)) return res;
  
  res = CreateBR(selNode, selOffset, outBRNode);
  if (NS_FAILED(res)) return res;
    
  // position selection after br
  res = GetNodeLocation(*outBRNode, address_of(selNode), &selOffset);
  if (NS_FAILED(res)) return res;
  nsCOMPtr<nsISelectionPrivate> selPriv(do_QueryInterface(selection));
  selPriv->SetInterlinePosition(PR_TRUE);
  return selection->Collapse(selNode, selOffset+1);
}

nsresult
nsPlaintextEditor::GetTextSelectionOffsets(nsISelection *aSelection,
                                           PRUint32 &aOutStartOffset, 
                                           PRUint32 &aOutEndOffset)
{
  NS_ASSERTION(aSelection, "null selection");

  nsresult rv;
  nsCOMPtr<nsIDOMNode> startNode, endNode;
  PRInt32 startNodeOffset, endNodeOffset;
  aSelection->GetAnchorNode(getter_AddRefs(startNode));
  aSelection->GetAnchorOffset(&startNodeOffset);
  aSelection->GetFocusNode(getter_AddRefs(endNode));
  aSelection->GetFocusOffset(&endNodeOffset);

  nsIDOMElement* rootNode = GetRoot();
  NS_ENSURE_TRUE(rootNode, NS_ERROR_NULL_POINTER);

  PRInt32 startOffset = -1;
  PRInt32 endOffset = -1;

  nsCOMPtr<nsIContentIterator> iter =
    do_CreateInstance("@mozilla.org/content/post-content-iterator;1", &rv);
  NS_ENSURE_SUCCESS(rv, rv);
    
#ifdef NS_DEBUG
  PRInt32 nodeCount = 0; // only needed for the assertions below
#endif
  PRUint32 totalLength = 0;
  nsCOMPtr<nsIContent> rootContent = do_QueryInterface(rootNode);
  iter->Init(rootContent);
  for (; !iter->IsDone() && (startOffset == -1 || endOffset == -1); iter->Next()) {
    nsCOMPtr<nsIDOMNode> currentNode = do_QueryInterface(iter->GetCurrentNode());
    nsCOMPtr<nsIDOMCharacterData> textNode = do_QueryInterface(currentNode);
    if (textNode) {
      // Note that sometimes we have an empty #text-node as start/endNode,
      // which we regard as not editable because the frame width == 0,
      // see nsEditor::IsEditable().
      PRBool editable = IsEditable(currentNode);
      if (currentNode == startNode) {
        startOffset = totalLength + (editable ? startNodeOffset : 0);
      }
      if (currentNode == endNode) {
        endOffset = totalLength + (editable ? endNodeOffset : 0);
      }
      if (editable) {
        PRUint32 length;
        textNode->GetLength(&length);
        totalLength += length;
      }
    }
#ifdef NS_DEBUG
    ++nodeCount;
#endif
  }

  if (endOffset == -1) {
    NS_ASSERTION(endNode == rootNode, "failed to find the end node");
    NS_ASSERTION(endNodeOffset == nodeCount-1 || endNodeOffset == 0,
                 "invalid end node offset");
    endOffset = endNodeOffset == 0 ? 0 : totalLength;
  }
  if (startOffset == -1) {
    NS_ASSERTION(startNode == rootNode, "failed to find the start node");
    NS_ASSERTION(startNodeOffset == nodeCount-1 || startNodeOffset == 0,
                 "invalid start node offset");
    startOffset = startNodeOffset == 0 ? 0 : totalLength;
  }

  // Make sure aOutStartOffset <= aOutEndOffset.
  if (startOffset <= endOffset) {
    aOutStartOffset = startOffset;
    aOutEndOffset = endOffset;
  }
  else {
    aOutStartOffset = endOffset;
    aOutEndOffset = startOffset;
  }

  return NS_OK;
}

NS_IMETHODIMP nsPlaintextEditor::DeleteSelection(nsIEditor::EDirection aAction)
{
  if (!mRules) { return NS_ERROR_NOT_INITIALIZED; }

  nsresult result;

  // delete placeholder txns merge.
  nsAutoPlaceHolderBatch batch(this, gDeleteTxnName);
  nsAutoRules beginRulesSniffing(this, kOpDeleteSelection, aAction);

  // pre-process
  nsCOMPtr<nsISelection> selection;
  result = GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(result)) return result;
  if (!selection) return NS_ERROR_NULL_POINTER;

  // If there is an existing selection when an extended delete is requested,
  //  platforms that use "caret-style" caret positioning collapse the
  //  selection to the  start and then create a new selection.
  //  Platforms that use "selection-style" caret positioning just delete the
  //  existing selection without extending it.
  PRBool bCollapsed;
  result  = selection->GetIsCollapsed(&bCollapsed);
  if (NS_FAILED(result)) return result;
  if (!bCollapsed &&
      (aAction == eNextWord || aAction == ePreviousWord ||
       aAction == eToBeginningOfLine || aAction == eToEndOfLine))
    if (mCaretStyle == 1)
    {
      result = selection->CollapseToStart();
      if (NS_FAILED(result)) return result;
    }
    else
    { 
      aAction = eNone;
    }

  // If it's one of these modes,
  // we have to extend the selection first.
  // This needs to happen inside selection batching,
  // otherwise the deleted text is autocopied to the clipboard.
  if (aAction == eNextWord || aAction == ePreviousWord
      || (aAction == eNext && bCollapsed)
      || aAction == eToBeginningOfLine || aAction == eToEndOfLine)
  {
    nsCOMPtr<nsISelectionController> selCont (do_QueryReferent(mSelConWeak));
    if (!selCont)
      return NS_ERROR_NO_INTERFACE;

    switch (aAction)
    {
        case eNextWord:
          result = selCont->WordExtendForDelete(PR_TRUE);
          // DeleteSelectionImpl doesn't handle these actions
          // because it's inside batching, so don't confuse it:
          aAction = eNone;
          break;
        case ePreviousWord:
          result = selCont->WordExtendForDelete(PR_FALSE);
          aAction = eNone;
          break;
        case eNext:
          result = selCont->CharacterExtendForDelete();
          aAction = eNone;
          break;
        case eToBeginningOfLine:
          selCont->IntraLineMove(PR_TRUE, PR_FALSE);          // try to move to end
          result = selCont->IntraLineMove(PR_FALSE, PR_TRUE); // select to beginning
          aAction = eNone;
          break;
        case eToEndOfLine:
          result = selCont->IntraLineMove(PR_TRUE, PR_TRUE);
          aAction = eNext;
          break;
        default:       // avoid several compiler warnings
          result = NS_OK;
          break;
    }
    NS_ENSURE_SUCCESS(result, result);
  }

  nsTextRulesInfo ruleInfo(nsTextEditRules::kDeleteSelection);
  ruleInfo.collapsedAction = aAction;
  PRBool cancel, handled;
  result = mRules->WillDoAction(selection, &ruleInfo, &cancel, &handled);
  if (NS_FAILED(result)) return result;
  if (!cancel && !handled)
  {
    result = DeleteSelectionImpl(aAction);
  }
  if (!cancel)
  {
    // post-process 
    result = mRules->DidDoAction(selection, &ruleInfo, result);
  }

  return result;
}

NS_IMETHODIMP nsPlaintextEditor::InsertText(const nsAString &aStringToInsert)
{
  if (!mRules) { return NS_ERROR_NOT_INITIALIZED; }

  PRInt32 theAction = nsTextEditRules::kInsertText;
  PRInt32 opID = kOpInsertText;
  if (mInIMEMode) 
  {
    theAction = nsTextEditRules::kInsertTextIME;
    opID = kOpInsertIMEText;
  }
  nsAutoPlaceHolderBatch batch(this, nsnull); 
  nsAutoRules beginRulesSniffing(this, opID, nsIEditor::eNext);

  // pre-process
  nsCOMPtr<nsISelection> selection;
  nsresult result = GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(result)) return result;
  if (!selection) return NS_ERROR_NULL_POINTER;
  nsAutoString resultString;
  // XXX can we trust instring to outlive ruleInfo,
  // XXX and ruleInfo not to refer to instring in its dtor?
  //nsAutoString instring(aStringToInsert);
  nsTextRulesInfo ruleInfo(theAction);
  ruleInfo.inString = &aStringToInsert;
  ruleInfo.outString = &resultString;
  ruleInfo.maxLength = mMaxTextLength;

  PRBool cancel, handled;
  result = mRules->WillDoAction(selection, &ruleInfo, &cancel, &handled);
  if (NS_FAILED(result)) return result;
  if (!cancel && !handled)
  {
    // we rely on rules code for now - no default implementation
  }
  if (!cancel)
  {
    // post-process 
    result = mRules->DidDoAction(selection, &ruleInfo, result);
  }
  return result;
}

NS_IMETHODIMP nsPlaintextEditor::InsertLineBreak()
{
  if (!mRules) { return NS_ERROR_NOT_INITIALIZED; }

  nsAutoEditBatch beginBatching(this);
  nsAutoRules beginRulesSniffing(this, kOpInsertBreak, nsIEditor::eNext);

  // pre-process
  nsCOMPtr<nsISelection> selection;
  nsresult res;
  res = GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(res)) return res;
  if (!selection) return NS_ERROR_NULL_POINTER;

  // Batching the selection and moving nodes out from under the caret causes
  // caret turds. Ask the shell to invalidate the caret now to avoid the turds.
  nsCOMPtr<nsIPresShell> shell;
  res = GetPresShell(getter_AddRefs(shell));
  if (NS_FAILED(res)) return res;
  shell->MaybeInvalidateCaretPosition();

  nsTextRulesInfo ruleInfo(nsTextEditRules::kInsertBreak);
  PRBool cancel, handled;
  res = mRules->WillDoAction(selection, &ruleInfo, &cancel, &handled);
  if (NS_FAILED(res)) return res;
  if (!cancel && !handled)
  {
    // create the new BR node
    nsCOMPtr<nsIDOMNode> newNode;
    res = DeleteSelectionAndCreateNode(NS_LITERAL_STRING("br"), getter_AddRefs(newNode));
    if (!newNode) res = NS_ERROR_NULL_POINTER; // don't return here, so DidDoAction is called
    if (NS_SUCCEEDED(res))
    {
      // set the selection to the new node
      nsCOMPtr<nsIDOMNode>parent;
      res = newNode->GetParentNode(getter_AddRefs(parent));
      if (!parent) res = NS_ERROR_NULL_POINTER; // don't return here, so DidDoAction is called
      if (NS_SUCCEEDED(res))
      {
        PRInt32 offsetInParent=-1;  // we use the -1 as a marker to see if we need to compute this or not
        nsCOMPtr<nsIDOMNode>nextNode;
        newNode->GetNextSibling(getter_AddRefs(nextNode));
        if (nextNode)
        {
          nsCOMPtr<nsIDOMCharacterData>nextTextNode = do_QueryInterface(nextNode);
          if (!nextTextNode) {
            nextNode = do_QueryInterface(newNode); // is this QI needed?
          }
          else { 
            offsetInParent=0; 
          }
        }
        else {
          nextNode = do_QueryInterface(newNode); // is this QI needed?
        }

        if (-1==offsetInParent) 
        {
          nextNode->GetParentNode(getter_AddRefs(parent));
          res = GetChildOffset(nextNode, parent, offsetInParent);
          if (NS_SUCCEEDED(res)) {
            // SetInterlinePosition(PR_TRUE) means we want the caret to stick to the content on the "right".
            // We want the caret to stick to whatever is past the break.  This is
            // because the break is on the same line we were on, but the next content
            // will be on the following line.
            nsCOMPtr<nsISelectionPrivate> selPriv(do_QueryInterface(selection));
            selPriv->SetInterlinePosition(PR_TRUE);
            res = selection->Collapse(parent, offsetInParent+1);  // +1 to insert just after the break
          }
        }
        else
        {
          res = selection->Collapse(nextNode, offsetInParent);
        }
      }
    }
  }
  if (!cancel)
  {
    // post-process, always called if WillInsertBreak didn't return cancel==PR_TRUE
    res = mRules->DidDoAction(selection, &ruleInfo, res);
  }

  return res;
}

NS_IMETHODIMP
nsPlaintextEditor::BeginComposition(nsTextEventReply* aReply)
{
  NS_ENSURE_TRUE(!mInIMEMode, NS_OK);

  if(mFlags & nsIPlaintextEditor::eEditorPasswordMask)  {
    if (mRules) {
      nsIEditRules *p = mRules.get();
      nsTextEditRules *textEditRules = static_cast<nsTextEditRules *>(p);
      textEditRules->ResetIMETextPWBuf();
    }
    else  {
      return NS_ERROR_NULL_POINTER;
    }
  }

  return nsEditor::BeginComposition(aReply);
}

NS_IMETHODIMP
nsPlaintextEditor::GetDocumentIsEmpty(PRBool *aDocumentIsEmpty)
{
  if (!aDocumentIsEmpty)
    return NS_ERROR_NULL_POINTER;
  
  if (!mRules)
    return NS_ERROR_NOT_INITIALIZED;
  
  return mRules->DocumentIsEmpty(aDocumentIsEmpty);
}

NS_IMETHODIMP
nsPlaintextEditor::GetTextLength(PRInt32 *aCount)
{
  NS_ASSERTION(aCount, "null pointer");

  // initialize out params
  *aCount = 0;
  
  // special-case for empty document, to account for the bogus node
  PRBool docEmpty;
  nsresult rv = GetDocumentIsEmpty(&docEmpty);
  NS_ENSURE_SUCCESS(rv, rv);
  if (docEmpty)
    return NS_OK;

  nsIDOMElement* rootNode = GetRoot();
  NS_ENSURE_TRUE(rootNode, NS_ERROR_NULL_POINTER);

  nsCOMPtr<nsIContentIterator> iter =
    do_CreateInstance("@mozilla.org/content/post-content-iterator;1", &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  PRUint32 totalLength = 0;
  nsCOMPtr<nsIContent> rootContent = do_QueryInterface(rootNode);
  iter->Init(rootContent);
  for (; !iter->IsDone(); iter->Next()) {
    nsCOMPtr<nsIDOMNode> currentNode = do_QueryInterface(iter->GetCurrentNode());
    nsCOMPtr<nsIDOMCharacterData> textNode = do_QueryInterface(currentNode);
    if (textNode && IsEditable(currentNode)) {
      PRUint32 length;
      textNode->GetLength(&length);
      totalLength += length;
    }
  }

  *aCount = totalLength;
  return NS_OK;
}

NS_IMETHODIMP
nsPlaintextEditor::SetMaxTextLength(PRInt32 aMaxTextLength)
{
  mMaxTextLength = aMaxTextLength;
  return NS_OK;
}

NS_IMETHODIMP
nsPlaintextEditor::GetMaxTextLength(PRInt32* aMaxTextLength)
{
  if (!aMaxTextLength)
    return NS_ERROR_INVALID_POINTER;
  *aMaxTextLength = mMaxTextLength;
  return NS_OK;
}

//
// Get the wrap width
//
NS_IMETHODIMP 
nsPlaintextEditor::GetWrapWidth(PRInt32 *aWrapColumn)
{
  if (! aWrapColumn)
    return NS_ERROR_NULL_POINTER;

  *aWrapColumn = mWrapColumn;
  return NS_OK;
}

//
// See if the style value includes this attribute, and if it does,
// cut out everything from the attribute to the next semicolon.
//
static void CutStyle(const char* stylename, nsString& styleValue)
{
  // Find the current wrapping type:
  PRInt32 styleStart = styleValue.Find(stylename, PR_TRUE);
  if (styleStart >= 0)
  {
    PRInt32 styleEnd = styleValue.Find(";", PR_FALSE, styleStart);
    if (styleEnd > styleStart)
      styleValue.Cut(styleStart, styleEnd - styleStart + 1);
    else
      styleValue.Cut(styleStart, styleValue.Length() - styleStart);
  }
}

//
// Change the wrap width on the root of this document.
// 
NS_IMETHODIMP 
nsPlaintextEditor::SetWrapWidth(PRInt32 aWrapColumn)
{
  SetWrapColumn(aWrapColumn);

  // Make sure we're a plaintext editor, otherwise we shouldn't
  // do the rest of this.
  PRUint32 flags = 0;
  GetFlags(&flags);
  if (!(flags & eEditorPlaintextMask))
    return NS_OK;

  // Ought to set a style sheet here ...
  // Probably should keep around an mPlaintextStyleSheet for this purpose.
  nsIDOMElement *rootElement = GetRoot();
  if (!rootElement)
    return NS_ERROR_NULL_POINTER;

  // Get the current style for this root element:
  NS_NAMED_LITERAL_STRING(styleName, "style");
  nsAutoString styleValue;
  nsresult res = rootElement->GetAttribute(styleName, styleValue);
  if (NS_FAILED(res)) return res;

  // We'll replace styles for these values:
  CutStyle("white-space", styleValue);
  CutStyle("width", styleValue);
  CutStyle("font-family", styleValue);

  // If we have other style left, trim off any existing semicolons
  // or whitespace, then add a known semicolon-space:
  if (!styleValue.IsEmpty())
  {
    styleValue.Trim("; \t", PR_FALSE, PR_TRUE);
    styleValue.AppendLiteral("; ");
  }

  // Make sure we have fixed-width font.  This should be done for us,
  // but it isn't, see bug 22502, so we have to add "font: -moz-fixed;".
  // Only do this if we're wrapping.
  if ((flags & eEditorEnableWrapHackMask) && aWrapColumn >= 0)
    styleValue.AppendLiteral("font-family: -moz-fixed; ");

  // If "mail.compose.wrap_to_window_width" is set, and we're a mail editor,
  // then remember our wrap width (for output purposes) but set the visual
  // wrapping to window width.
  // We may reset mWrapToWindow here, based on the pref's current value.
  if (flags & eEditorMailMask)
  {
    nsresult rv;
    nsCOMPtr<nsIPrefBranch> prefBranch =
      do_GetService(NS_PREFSERVICE_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv))
      prefBranch->GetBoolPref("mail.compose.wrap_to_window_width",
                              &mWrapToWindow);
  }

  // and now we're ready to set the new whitespace/wrapping style.
  if (aWrapColumn > 0 && !mWrapToWindow)        // Wrap to a fixed column
  {
    styleValue.AppendLiteral("white-space: pre-wrap; width: ");
    styleValue.AppendInt(aWrapColumn);
    styleValue.AppendLiteral("ch;");
  }
  else if (mWrapToWindow || aWrapColumn == 0)
    styleValue.AppendLiteral("white-space: pre-wrap;");
  else
    styleValue.AppendLiteral("white-space: pre;");

  return rootElement->SetAttribute(styleName, styleValue);
}

NS_IMETHODIMP 
nsPlaintextEditor::SetWrapColumn(PRInt32 aWrapColumn)
{
  mWrapColumn = aWrapColumn;
  return NS_OK;
}

//
// Get the newline handling for this editor
//
NS_IMETHODIMP 
nsPlaintextEditor::GetNewlineHandling(PRInt32 *aNewlineHandling)
{
  NS_ENSURE_ARG_POINTER(aNewlineHandling);

  *aNewlineHandling = mNewlineHandling;
  return NS_OK;
}

//
// Change the newline handling for this editor
// 
NS_IMETHODIMP 
nsPlaintextEditor::SetNewlineHandling(PRInt32 aNewlineHandling)
{
  mNewlineHandling = aNewlineHandling;
  
  return NS_OK;
}

#ifdef XP_MAC
#pragma mark -
#pragma mark  nsIEditor overrides 
#pragma mark -
#endif

NS_IMETHODIMP 
nsPlaintextEditor::Undo(PRUint32 aCount)
{
  nsAutoUpdateViewBatch beginViewBatching(this);

  ForceCompositionEnd();

  nsAutoRules beginRulesSniffing(this, kOpUndo, nsIEditor::eNone);

  nsTextRulesInfo ruleInfo(nsTextEditRules::kUndo);
  nsCOMPtr<nsISelection> selection;
  GetSelection(getter_AddRefs(selection));
  PRBool cancel, handled;
  nsresult result = mRules->WillDoAction(selection, &ruleInfo, &cancel, &handled);
  
  if (!cancel && NS_SUCCEEDED(result))
  {
    result = nsEditor::Undo(aCount);
    result = mRules->DidDoAction(selection, &ruleInfo, result);
  } 
   
  return result;
}

NS_IMETHODIMP 
nsPlaintextEditor::Redo(PRUint32 aCount)
{
  nsAutoUpdateViewBatch beginViewBatching(this);

  ForceCompositionEnd();

  nsAutoRules beginRulesSniffing(this, kOpRedo, nsIEditor::eNone);

  nsTextRulesInfo ruleInfo(nsTextEditRules::kRedo);
  nsCOMPtr<nsISelection> selection;
  GetSelection(getter_AddRefs(selection));
  PRBool cancel, handled;
  nsresult result = mRules->WillDoAction(selection, &ruleInfo, &cancel, &handled);
  
  if (!cancel && NS_SUCCEEDED(result))
  {
    result = nsEditor::Redo(aCount);
    result = mRules->DidDoAction(selection, &ruleInfo, result);
  } 
   
  return result;
}

nsresult nsPlaintextEditor::GetClipboardEventTarget(nsIDOMNode** aEventTarget)
{
  NS_ENSURE_ARG_POINTER(aEventTarget);
  *aEventTarget = nsnull;

  nsCOMPtr<nsISelection> selection;
  nsresult res = GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(res))
    return res;

  return nsCopySupport::GetClipboardEventTarget(selection, aEventTarget);
}

nsresult nsPlaintextEditor::FireClipboardEvent(PRUint32 msg,
                                               PRBool* aPreventDefault)
{
  *aPreventDefault = PR_FALSE;

  nsCOMPtr<nsIPresShell> ps = do_QueryReferent(mPresShellWeak);
  if (!ps)
    return NS_ERROR_NOT_INITIALIZED;

  // Unsafe to fire event during reflow (bug 396108)
  PRBool isReflowing = PR_TRUE;
  nsresult rv = ps->IsReflowLocked(&isReflowing);
  if (NS_FAILED(rv) || isReflowing)
    return NS_OK;

  nsCOMPtr<nsIDOMNode> eventTarget;
  rv = GetClipboardEventTarget(getter_AddRefs(eventTarget));
  if (NS_FAILED(rv))
    // On failure to get event target, just forget about it and don't fire.
    return NS_OK;

  nsEventStatus status = nsEventStatus_eIgnore;
  nsEvent evt(PR_TRUE, msg);
  nsEventDispatcher::Dispatch(eventTarget, ps->GetPresContext(), &evt,
                              nsnull, &status);
  // if event handler return'd false (PreventDefault)
  if (status == nsEventStatus_eConsumeNoDefault)
    *aPreventDefault = PR_TRUE;

  // Did the event handler cause the editor to be destroyed? (ie. the input
  // element was removed from the document)  Don't proceed with command,
  // could crash, definitely does during paste.
  if (mDidPreDestroy)
    return NS_ERROR_NOT_INITIALIZED;

  return NS_OK;
}

NS_IMETHODIMP nsPlaintextEditor::Cut()
{
  PRBool preventDefault;
  nsresult rv = FireClipboardEvent(NS_CUT, &preventDefault);
  if (NS_FAILED(rv) || preventDefault)
    return rv;

  nsCOMPtr<nsISelection> selection;
  rv = GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(rv))
    return rv;

  PRBool isCollapsed;
  if (NS_SUCCEEDED(selection->GetIsCollapsed(&isCollapsed)) && isCollapsed)
    return NS_OK;  // just return ok so no JS error is thrown

  // ps should be guaranteed by FireClipboardEvent not failing
  nsCOMPtr<nsIPresShell> ps = do_QueryReferent(mPresShellWeak);
  rv = ps->DoCopy();
  if (NS_SUCCEEDED(rv))
    rv = DeleteSelection(eNone);
  return rv;
}

NS_IMETHODIMP nsPlaintextEditor::CanCut(PRBool *aCanCut)
{
  NS_ENSURE_ARG_POINTER(aCanCut);
  *aCanCut = PR_FALSE;

  nsCOMPtr<nsISelection> selection;
  nsresult rv = GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(rv)) return rv;
    
  PRBool isCollapsed;
  rv = selection->GetIsCollapsed(&isCollapsed);
  if (NS_FAILED(rv)) return rv;

  *aCanCut = !isCollapsed && IsModifiable();
  return NS_OK;
}

NS_IMETHODIMP nsPlaintextEditor::Copy()
{
  PRBool preventDefault;
  nsresult rv = FireClipboardEvent(NS_COPY, &preventDefault);
  if (NS_FAILED(rv) || preventDefault)
    return rv;

  // ps should be guaranteed by FireClipboardEvent not failing
  nsCOMPtr<nsIPresShell> ps = do_QueryReferent(mPresShellWeak);
  return ps->DoCopy();
}

NS_IMETHODIMP nsPlaintextEditor::CanCopy(PRBool *aCanCopy)
{
  NS_ENSURE_ARG_POINTER(aCanCopy);
  *aCanCopy = PR_FALSE;

  nsCOMPtr<nsISelection> selection;
  nsresult rv = GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(rv)) return rv;
    
  PRBool isCollapsed;
  rv = selection->GetIsCollapsed(&isCollapsed);
  if (NS_FAILED(rv)) return rv;

  *aCanCopy = !isCollapsed;
  return NS_OK;
}


// Shared between OutputToString and OutputToStream
NS_IMETHODIMP
nsPlaintextEditor::GetAndInitDocEncoder(const nsAString& aFormatType,
                                        PRUint32 aFlags,
                                        const nsACString& aCharset,
                                        nsIDocumentEncoder** encoder)
{
  nsCOMPtr<nsIPresShell> presShell;
  nsresult rv = GetPresShell(getter_AddRefs(presShell));
  if (NS_FAILED(rv)) return rv;
  if (!presShell) return NS_ERROR_FAILURE;

  nsCAutoString formatType(NS_DOC_ENCODER_CONTRACTID_BASE);
  formatType.AppendWithConversion(aFormatType);
  nsCOMPtr<nsIDocumentEncoder> docEncoder (do_CreateInstance(formatType.get(), &rv));
  NS_ENSURE_SUCCESS(rv, rv);

  nsIDocument *doc = presShell->GetDocument();
  nsCOMPtr<nsIDOMDocument> domDoc = do_QueryInterface(doc);
  NS_ASSERTION(domDoc, "Need a document");

  rv = docEncoder->Init(domDoc, aFormatType, aFlags);
  NS_ENSURE_SUCCESS(rv, rv);

  if (!aCharset.IsEmpty()
    && !(aCharset.EqualsLiteral("null")))
    docEncoder->SetCharset(aCharset);

  PRInt32 wc;
  (void) GetWrapWidth(&wc);
  if (wc >= 0)
    (void) docEncoder->SetWrapColumn(wc);

  // Set the selection, if appropriate.
  // We do this either if the OutputSelectionOnly flag is set,
  // in which case we use our existing selection ...
  if (aFlags & nsIDocumentEncoder::OutputSelectionOnly)
  {
    nsCOMPtr<nsISelection> selection;
    rv = GetSelection(getter_AddRefs(selection));
    if (NS_SUCCEEDED(rv) && selection)
      rv = docEncoder->SetSelection(selection);
    NS_ENSURE_SUCCESS(rv, rv);
  }
  // ... or if the root element is not a body,
  // in which case we set the selection to encompass the root.
  else
  {
    nsIDOMElement *rootElement = GetRoot();
    NS_ENSURE_TRUE(rootElement, NS_ERROR_FAILURE);
    if (!nsTextEditUtils::IsBody(rootElement))
    {
      rv = docEncoder->SetContainerNode(rootElement);
      NS_ENSURE_SUCCESS(rv, rv);
    }
  }

  NS_ADDREF(*encoder = docEncoder);
  return rv;
}


NS_IMETHODIMP 
nsPlaintextEditor::OutputToString(const nsAString& aFormatType,
                                  PRUint32 aFlags,
                                  nsAString& aOutputString)
{
  nsString resultString;
  nsTextRulesInfo ruleInfo(nsTextEditRules::kOutputText);
  ruleInfo.outString = &resultString;
  // XXX Struct should store a nsAReadable*
  nsAutoString str(aFormatType);
  ruleInfo.outputFormat = &str;
  PRBool cancel, handled;
  nsresult rv = mRules->WillDoAction(nsnull, &ruleInfo, &cancel, &handled);
  if (cancel || NS_FAILED(rv)) { return rv; }
  if (handled)
  { // this case will get triggered by password fields
    aOutputString.Assign(*(ruleInfo.outString));
    return rv;
  }

  nsCAutoString charsetStr;
  rv = GetDocumentCharacterSet(charsetStr);
  if(NS_FAILED(rv) || charsetStr.IsEmpty())
    charsetStr.AssignLiteral("ISO-8859-1");

  nsCOMPtr<nsIDocumentEncoder> encoder;
  rv = GetAndInitDocEncoder(aFormatType, aFlags, charsetStr, getter_AddRefs(encoder));
  if (NS_FAILED(rv))
    return rv;
  return encoder->EncodeToString(aOutputString);
}

NS_IMETHODIMP
nsPlaintextEditor::OutputToStream(nsIOutputStream* aOutputStream,
                             const nsAString& aFormatType,
                             const nsACString& aCharset,
                             PRUint32 aFlags)
{
  nsresult rv;

  // special-case for empty document when requesting plain text,
  // to account for the bogus text node.
  // XXX Should there be a similar test in OutputToString?
  if (aFormatType.EqualsLiteral("text/plain"))
  {
    PRBool docEmpty;
    rv = GetDocumentIsEmpty(&docEmpty);
    if (NS_FAILED(rv)) return rv;
    
    if (docEmpty)
       return NS_OK;    // output nothing
  }

  nsCOMPtr<nsIDocumentEncoder> encoder;
  rv = GetAndInitDocEncoder(aFormatType, aFlags, aCharset,
                            getter_AddRefs(encoder));

  if (NS_FAILED(rv))
    return rv;

  return encoder->EncodeToStream(aOutputStream);
}


#ifdef XP_MAC
#pragma mark -
#pragma mark  nsIEditorMailSupport overrides 
#pragma mark -
#endif

NS_IMETHODIMP
nsPlaintextEditor::InsertTextWithQuotations(const nsAString &aStringToInsert)
{
  return InsertText(aStringToInsert);
}

NS_IMETHODIMP
nsPlaintextEditor::PasteAsQuotation(PRInt32 aSelectionType)
{
  // Get Clipboard Service
  nsresult rv;
  nsCOMPtr<nsIClipboard> clipboard(do_GetService("@mozilla.org/widget/clipboard;1", &rv));
  if (NS_FAILED(rv)) return rv;

  // Create generic Transferable for getting the data
  nsCOMPtr<nsITransferable> trans = do_CreateInstance("@mozilla.org/widget/transferable;1", &rv);
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
    PRUint32 len;
    char* flav = nsnull;
    rv = trans->GetAnyTransferData(&flav, getter_AddRefs(genericDataObj),
                                   &len);
    if (NS_FAILED(rv) || !flav)
    {
#ifdef DEBUG_akkana
      printf("PasteAsPlaintextQuotation: GetAnyTransferData failed, %d\n", rv);
#endif
      return rv;
    }
#ifdef DEBUG_clipboard
    printf("Got flavor [%s]\n", flav);
#endif
    if (0 == nsCRT::strcmp(flav, kUnicodeMime))
    {
      nsCOMPtr<nsISupportsString> textDataObj ( do_QueryInterface(genericDataObj) );
      if (textDataObj && len > 0)
      {
        nsAutoString stuffToPaste;
        textDataObj->GetData ( stuffToPaste );
        nsAutoEditBatch beginBatching(this);
        rv = InsertAsQuotation(stuffToPaste, 0);
      }
    }
    NS_Free(flav);
  }

  return rv;
}

// Utility routine to make a new citer.  This addrefs, of course.
static nsICiter* MakeACiter()
{
  // Make a citer of an appropriate type
  nsICiter* citer = 0;
  nsresult rv;
  nsCOMPtr<nsIPrefBranch> prefBranch =
    do_GetService(NS_PREFSERVICE_CONTRACTID, &rv);
  if (NS_FAILED(rv)) return 0;

  char *citationType = 0;
  rv = prefBranch->GetCharPref("mail.compose.citationType", &citationType);
                          
  if (NS_SUCCEEDED(rv) && citationType[0] && !strncmp(citationType, "aol", 3))
    citer = new nsAOLCiter;
  else
    citer = new nsInternetCiter;

  if (citationType)
    PL_strfree(citationType);

  if (citer)
    NS_ADDREF(citer);
  return citer;
}

NS_IMETHODIMP
nsPlaintextEditor::InsertAsQuotation(const nsAString& aQuotedText,
                                     nsIDOMNode **aNodeInserted)
{
  // We have the text.  Cite it appropriately:
  nsCOMPtr<nsICiter> citer = dont_AddRef(MakeACiter());

  // Let the citer quote it for us:
  nsString quotedStuff;
  nsresult rv = citer->GetCiteString(aQuotedText, quotedStuff);
  if (NS_FAILED(rv))
    return rv;

  // It's best to put a blank line after the quoted text so that mails
  // written without thinking won't be so ugly.
  if (!aQuotedText.IsEmpty() && (aQuotedText.Last() != PRUnichar('\n')))
    quotedStuff.Append(PRUnichar('\n'));

  // get selection
  nsCOMPtr<nsISelection> selection;
  rv = GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(rv)) return rv;
  if (!selection) return NS_ERROR_NULL_POINTER;

  nsAutoEditBatch beginBatching(this);
  nsAutoRules beginRulesSniffing(this, kOpInsertText, nsIEditor::eNext);

  // give rules a chance to handle or cancel
  nsTextRulesInfo ruleInfo(nsTextEditRules::kInsertElement);
  PRBool cancel, handled;
  rv = mRules->WillDoAction(selection, &ruleInfo, &cancel, &handled);
  if (NS_FAILED(rv)) return rv;
  if (cancel) return NS_OK; // rules canceled the operation
  if (!handled)
  {
    rv = InsertText(quotedStuff);

    // XXX Should set *aNodeInserted to the first node inserted
    if (aNodeInserted && NS_SUCCEEDED(rv))
    {
      *aNodeInserted = 0;
      //NS_IF_ADDREF(*aNodeInserted);
    }
  }
  return rv;
}

NS_IMETHODIMP
nsPlaintextEditor::PasteAsCitedQuotation(const nsAString& aCitation,
                                         PRInt32 aSelectionType)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsPlaintextEditor::InsertAsCitedQuotation(const nsAString& aQuotedText,
                                          const nsAString& aCitation,
                                          PRBool aInsertHTML,
                                          nsIDOMNode **aNodeInserted)
{
  return InsertAsQuotation(aQuotedText, aNodeInserted);
}

nsresult
nsPlaintextEditor::SharedOutputString(PRUint32 aFlags,
                                      PRBool* aIsCollapsed,
                                      nsAString& aResult)
{
  nsCOMPtr<nsISelection> selection;
  nsresult rv = GetSelection(getter_AddRefs(selection));
  NS_ENSURE_SUCCESS(rv, rv);
  if (!selection)
    return NS_ERROR_NOT_INITIALIZED;

  rv = selection->GetIsCollapsed(aIsCollapsed);
  NS_ENSURE_SUCCESS(rv, rv);

  if (!*aIsCollapsed)
    aFlags |= nsIDocumentEncoder::OutputSelectionOnly;
  // If the selection isn't collapsed, we'll use the whole document.

  return OutputToString(NS_LITERAL_STRING("text/plain"), aFlags, aResult);
}

NS_IMETHODIMP
nsPlaintextEditor::Rewrap(PRBool aRespectNewlines)
{
  PRInt32 wrapCol;
  nsresult rv = GetWrapWidth(&wrapCol);
  if (NS_FAILED(rv))
    return NS_OK;

  // Rewrap makes no sense if there's no wrap column; default to 72.
  if (wrapCol <= 0)
    wrapCol = 72;

#ifdef DEBUG_akkana
  printf("nsPlaintextEditor::Rewrap to %ld columns\n", (long)wrapCol);
#endif

  nsAutoString current;
  PRBool isCollapsed;
  rv = SharedOutputString(nsIDocumentEncoder::OutputFormatted
                          | nsIDocumentEncoder::OutputLFLineBreak,
                          &isCollapsed, current);
  if (NS_FAILED(rv)) return rv;

  nsCOMPtr<nsICiter> citer = dont_AddRef(MakeACiter());
  if (NS_FAILED(rv)) return rv;
  if (!citer) return NS_ERROR_UNEXPECTED;

  nsString wrapped;
  PRUint32 firstLineOffset = 0;   // XXX need to reset this if there is a selection
  rv = citer->Rewrap(current, wrapCol, firstLineOffset, aRespectNewlines,
                     wrapped);
  if (NS_FAILED(rv)) return rv;

  if (isCollapsed)    // rewrap the whole document
    SelectAll();

  return InsertTextWithQuotations(wrapped);
}

NS_IMETHODIMP    
nsPlaintextEditor::StripCites()
{
#ifdef DEBUG_akkana
  printf("nsPlaintextEditor::StripCites()\n");
#endif

  nsAutoString current;
  PRBool isCollapsed;
  nsresult rv = SharedOutputString(nsIDocumentEncoder::OutputFormatted,
                                   &isCollapsed, current);
  if (NS_FAILED(rv)) return rv;

  nsCOMPtr<nsICiter> citer = dont_AddRef(MakeACiter());
  if (!citer) return NS_ERROR_UNEXPECTED;

  nsString stripped;
  rv = citer->StripCites(current, stripped);
  if (NS_FAILED(rv)) return rv;

  if (isCollapsed)    // rewrap the whole document
  {
    rv = SelectAll();
    if (NS_FAILED(rv)) return rv;
  }

  return InsertText(stripped);
}

NS_IMETHODIMP
nsPlaintextEditor::GetEmbeddedObjects(nsISupportsArray** aNodeList)
{
  *aNodeList = 0;
  return NS_OK;
}


#ifdef XP_MAC
#pragma mark -
#pragma mark  nsIEditorIMESupport overrides 
#pragma mark -
#endif

NS_IMETHODIMP
nsPlaintextEditor::SetCompositionString(const nsAString& aCompositionString, nsIPrivateTextRangeList* aTextRangeList,nsTextEventReply* aReply)
{
  if (!aTextRangeList && !aCompositionString.IsEmpty())
  {
    NS_ERROR("aTextRangeList is null but the composition string is not null");
    return NS_ERROR_NULL_POINTER;
  }

  nsCOMPtr<nsIPresShell> ps = do_QueryReferent(mPresShellWeak);
  if (!ps) 
    return NS_ERROR_NOT_INITIALIZED;

  nsCOMPtr<nsISelection> selection;
  nsresult result = GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(result)) return result;

  nsCOMPtr<nsICaret>  caretP;
  ps->GetCaret(getter_AddRefs(caretP));

  // We should return caret position if it is possible. Because this event
  // dispatcher always expects to be returned the correct caret position.
  // But in following cases, we don't need to process the composition string,
  // so, we only need to return the caret position.

  // aCompositionString.IsEmpty() && !mIMETextNode:
  //   Workaround for Windows IME bug 23558: We get every IME event twice.
  //   For escape keypress, this causes an empty string to be passed
  //   twice, which freaks out the editor.

  // aCompositionString.IsEmpty() && !aTextRangeList:
  //   Some Chinese IMEs for Linux are always composition string and text range
  //   list are empty when listing the Chinese characters. In this case,
  //   we don't need to process composition string too. See bug 271815.

  if (!aCompositionString.IsEmpty() || (mIMETextNode && aTextRangeList))
  {
    mIMETextRangeList = aTextRangeList;

    // XXX_kin: BEGIN HACK! HACK! HACK!
    // XXX_kin:
    // XXX_kin: This is lame! The IME stuff needs caret coordinates
    // XXX_kin: synchronously, but the editor could be using async
    // XXX_kin: updates (reflows and paints) for performance reasons.
    // XXX_kin: In order to give IME what it needs, we have to temporarily
    // XXX_kin: switch to sync updating during this call so that the
    // XXX_kin: nsAutoPlaceHolderBatch can force sync reflows, paints,
    // XXX_kin: and selection scrolling, so that we get back accurate
    // XXX_kin: caret coordinates.

    PRUint32 flags = 0;
    PRBool restoreFlags = PR_FALSE;

    if (NS_SUCCEEDED(GetFlags(&flags)) &&
        (flags & nsIPlaintextEditor::eEditorUseAsyncUpdatesMask))
    {
      if (NS_SUCCEEDED(SetFlags(
          flags & (~nsIPlaintextEditor::eEditorUseAsyncUpdatesMask))))
        restoreFlags = PR_TRUE;
    }

    // XXX_kin: END HACK! HACK! HACK!

    // we need the nsAutoPlaceHolderBatch destructor called before hitting
    // GetCaretCoordinates so the states in Frame system sync with content
    // therefore, we put the nsAutoPlaceHolderBatch into a inner block
    {
      nsAutoPlaceHolderBatch batch(this, gIMETxnName);

      SetIsIMEComposing(); // We set mIsIMEComposing properly.

      result = InsertText(aCompositionString);

      mIMEBufferLength = aCompositionString.Length();

      if (caretP)
        caretP->SetCaretDOMSelection(selection);

      // second part of 23558 fix:
      if (aCompositionString.IsEmpty())
        mIMETextNode = nsnull;
    }

    // XXX_kin: BEGIN HACK! HACK! HACK!
    // XXX_kin:
    // XXX_kin: Restore the previous set of flags!

    if (restoreFlags)
      SetFlags(flags);

    // XXX_kin: END HACK! HACK! HACK!
  }

  if (caretP)
  {
    result = caretP->GetCaretCoordinates(nsICaret::eIMECoordinates,
                                         selection,
                                         &(aReply->mCursorPosition),
                                         &(aReply->mCursorIsCollapsed),
                                         nsnull);
    NS_ASSERTION(NS_SUCCEEDED(result), "cannot get caret position");
  }

  return result;
}

NS_IMETHODIMP 
nsPlaintextEditor::GetReconversionString(nsReconversionEventReply* aReply)
{
  nsCOMPtr<nsISelection> selection;
  nsresult res = GetSelection(getter_AddRefs(selection));
  if (NS_FAILED(res)) return res;
  if (!selection) return NS_ERROR_FAILURE;

  // XXX get the first range in the selection.  Since it is
  // unclear what to do if reconversion happens with a 
  // multirange selection, we will ignore any additional ranges.
  
  nsCOMPtr<nsIDOMRange> range;
  res = selection->GetRangeAt(0, getter_AddRefs(range));
  if (NS_FAILED(res)) return res;
  if (!range) return NS_ERROR_FAILURE;
  
  nsAutoString textValue;
  res = range->ToString(textValue);
  if (NS_FAILED(res))
    return res;
  
  aReply->mReconversionString = (PRUnichar*) nsMemory::Clone(textValue.get(),
                                                                (textValue.Length() + 1) * sizeof(PRUnichar));
  if (!aReply->mReconversionString)
    return NS_ERROR_OUT_OF_MEMORY;

  if (textValue.IsEmpty())
    return NS_OK;

  // delete the selection
  return DeleteSelection(eNone);
}

#ifdef XP_MAC
#pragma mark -
#pragma mark  nsEditor overrides 
#pragma mark -
#endif


/** All editor operations which alter the doc should be prefaced
 *  with a call to StartOperation, naming the action and direction */
NS_IMETHODIMP
nsPlaintextEditor::StartOperation(PRInt32 opID, nsIEditor::EDirection aDirection)
{
  nsEditor::StartOperation(opID, aDirection);  // will set mAction, mDirection
  if (mRules) return mRules->BeforeEdit(mAction, mDirection);
  return NS_OK;
}


/** All editor operations which alter the doc should be followed
 *  with a call to EndOperation */
NS_IMETHODIMP
nsPlaintextEditor::EndOperation()
{
  // post processing
  nsresult res = NS_OK;
  if (mRules) res = mRules->AfterEdit(mAction, mDirection);
  nsEditor::EndOperation();  // will clear mAction, mDirection
  return res;
}  


NS_IMETHODIMP 
nsPlaintextEditor::SelectEntireDocument(nsISelection *aSelection)
{
  if (!aSelection || !mRules) { return NS_ERROR_NULL_POINTER; }

  // is doc empty?
  PRBool bDocIsEmpty;
  if (NS_SUCCEEDED(mRules->DocumentIsEmpty(&bDocIsEmpty)) && bDocIsEmpty)
  {
    // get root node
    nsIDOMElement *rootElement = GetRoot();
    if (!rootElement)
      return NS_ERROR_FAILURE;

    // if it's empty don't select entire doc - that would select the bogus node
    return aSelection->Collapse(rootElement, 0);
  }

  return nsEditor::SelectEntireDocument(aSelection);
}



#ifdef XP_MAC
#pragma mark -
#pragma mark  Random methods 
#pragma mark -
#endif


NS_IMETHODIMP nsPlaintextEditor::GetLayoutObject(nsIDOMNode *aNode, nsISupports **aLayoutObject)
{
  nsCOMPtr<nsIPresShell> ps = do_QueryReferent(mPresShellWeak);
  if (!ps) return NS_ERROR_NOT_INITIALIZED;

  nsresult result = NS_ERROR_NULL_POINTER;
  if (aNode)
  { // get the content interface
    nsCOMPtr<nsIContent> nodeAsContent( do_QueryInterface(aNode) );
    if (nodeAsContent)
    { // get the frame from the content interface
      //Note: frames are not ref counted, so don't use an nsCOMPtr
      *aLayoutObject = nsnull;
      result = ps->GetLayoutObjectFor(nodeAsContent, aLayoutObject);
    }
  }

  return result;
}

#ifdef XP_MAC
#pragma mark -
#endif

nsresult
nsPlaintextEditor::SetAttributeOrEquivalent(nsIDOMElement * aElement,
                                            const nsAString & aAttribute,
                                            const nsAString & aValue,
                                            PRBool aSuppressTransaction)
{
  return nsEditor::SetAttribute(aElement, aAttribute, aValue);
}

nsresult
nsPlaintextEditor::RemoveAttributeOrEquivalent(nsIDOMElement * aElement,
                                               const nsAString & aAttribute,
                                               PRBool aSuppressTransaction)
{
  return nsEditor::RemoveAttribute(aElement, aAttribute);
}
