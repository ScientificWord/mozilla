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

#ifndef __editor_h__
#define __editor_h__

#include "nsCOMPtr.h"
#include "nsWeakReference.h"

#include "nsIEditor.h"
#include "nsIEditorIMESupport.h"
#include "nsIPhonetic.h"
#include "nsIKBStateControl.h"

#include "nsIAtom.h"
#include "nsIDOMDocument.h"
#include "nsISelection.h"
#include "nsIDOMCharacterData.h"
#include "nsIPrivateTextRange.h"
#include "nsITransactionManager.h"
#include "nsIComponentManager.h"
#include "nsISupportsArray.h"
#include "nsICSSStyleSheet.h"
#include "nsIDOMElement.h"
#include "nsSelectionState.h"
#include "nsIEditorSpellCheck.h"
#include "nsIInlineSpellChecker.h"

class nsIEditActionListener;
class nsIDocumentStateListener;
class nsIDOMCharacterData;
class nsIDOMRange;
class nsIPresShell;
class nsIViewManager;
class ChangeAttributeTxn;
class CreateElementTxn;
class InsertElementTxn;
class DeleteElementTxn;
class InsertTextTxn;
class DeleteTextTxn;
class SplitElementTxn;
class JoinElementTxn;
class EditAggregateTxn;
class nsVoidArray;
class nsISupportsArray;
class nsILocale;
class IMETextTxn;
class AddStyleSheetTxn;
class RemoveStyleSheetTxn;
class nsIFile;
class nsISelectionController;
class nsIDOMEventReceiver;

#define kMOZEditorBogusNodeAttr NS_LITERAL_STRING("_moz_editor_bogus_node")
#define kMOZEditorBogusNodeValue NS_LITERAL_STRING("TRUE")

/** implementation of an editor object.  it will be the controller/focal point 
 *  for the main editor services. i.e. the GUIManager, publishing, transaction 
 *  manager, event interfaces. the idea for the event interfaces is to have them 
 *  delegate the actual commands to the editor independent of the XPFE implementation.
 */
class nsEditor : public nsIEditor,
                 public nsIEditorIMESupport,
                 public nsSupportsWeakReference,
                 public nsIPhonetic
{
public:

  enum IterDirection
  {
    kIterForward,
    kIterBackward
  };

  enum OperationID
  {
    kOpIgnore = -1,
    kOpNone = 0,
    kOpUndo,
    kOpRedo,
    kOpInsertNode,
    kOpCreateNode,
    kOpDeleteNode,
    kOpSplitNode,
    kOpJoinNode,
    kOpDeleteSelection,
    // text commands
    kOpInsertBreak    = 1000,
    kOpInsertText     = 1001,
    kOpInsertIMEText  = 1002,
    kOpDeleteText     = 1003
  };

  /** The default constructor. This should suffice. the setting of the interfaces is done
   *  after the construction of the editor class.
   */
  nsEditor();
  /** The default destructor. This should suffice. Should this be pure virtual 
   *  for someone to derive from the nsEditor later? I don't believe so.
   */
  virtual ~nsEditor();

//Interfaces for addref and release and queryinterface
//NOTE: Use   NS_DECL_ISUPPORTS_INHERITED in any class inherited from nsEditor
  NS_DECL_ISUPPORTS

  /* ------------ utility methods   -------------- */
  NS_IMETHOD GetPresShell(nsIPresShell **aPS);
  void NotifyEditorObservers(void);

  /* ------------ nsIEditor methods -------------- */
  NS_DECL_NSIEDITOR
  /* ------------ nsIEditorIMESupport methods -------------- */
  NS_DECL_NSIEDITORIMESUPPORT
  
  // nsIPhonetic
  NS_DECL_NSIPHONETIC


public:

  
  NS_IMETHOD InsertTextImpl(const nsAString& aStringToInsert, 
                               nsCOMPtr<nsIDOMNode> *aInOutNode, 
                               PRInt32 *aInOutOffset,
                               nsIDOMDocument *aDoc);
  NS_IMETHOD InsertTextIntoTextNodeImpl(const nsAString& aStringToInsert, 
                                           nsIDOMCharacterData *aTextNode, 
                                           PRInt32 aOffset, PRBool suppressIME=PR_FALSE);
  NS_IMETHOD DeleteSelectionImpl(EDirection aAction);
  NS_IMETHOD DeleteSelectionAndCreateNode(const nsAString& aTag,
                                           nsIDOMNode ** aNewNode);

  /* helper routines for node/parent manipulations */
  nsresult ReplaceContainer(nsIDOMNode *inNode, 
                            nsCOMPtr<nsIDOMNode> *outNode, 
                            const nsAString &aNodeType,
                            const nsAString *aAttribute = nsnull,
                            const nsAString *aValue = nsnull,
                            PRBool aCloneAttributes = PR_FALSE);

  nsresult RemoveContainer(nsIDOMNode *inNode);
  nsresult InsertContainerAbove(nsIDOMNode *inNode, 
                                nsCOMPtr<nsIDOMNode> *outNode, 
                                const nsAString &aNodeType,
                                const nsAString *aAttribute = nsnull,
                                const nsAString *aValue = nsnull);
  nsresult MoveNode(nsIDOMNode *aNode, nsIDOMNode *aParent, PRInt32 aOffset);

  /* Method to replace certain CreateElementNS() calls. 
     Arguments:
      nsString& aTag          - tag you want
      nsIContent** aContent   - returned Content that was created with above namespace.
  */
  nsresult CreateHTMLContent(const nsAString& aTag, nsIContent** aContent);

protected:
  nsCString mContentMIMEType;       // MIME type of the doc we are editing.

  /** create a transaction for setting aAttribute to aValue on aElement
    */
  NS_IMETHOD CreateTxnForSetAttribute(nsIDOMElement *aElement, 
                                      const nsAString &  aAttribute, 
                                      const nsAString &  aValue,
                                      ChangeAttributeTxn ** aTxn);

  /** create a transaction for removing aAttribute on aElement
    */
  NS_IMETHOD CreateTxnForRemoveAttribute(nsIDOMElement *aElement, 
                                         const nsAString &  aAttribute,
                                         ChangeAttributeTxn ** aTxn);

  /** create a transaction for creating a new child node of aParent of type aTag.
    */
  NS_IMETHOD CreateTxnForCreateElement(const nsAString & aTag,
                                       nsIDOMNode     *aParent,
                                       PRInt32         aPosition,
                                       CreateElementTxn ** aTxn);

  /** create a transaction for inserting aNode as a child of aParent.
    */
  NS_IMETHOD CreateTxnForInsertElement(nsIDOMNode * aNode,
                                       nsIDOMNode * aParent,
                                       PRInt32      aOffset,
                                       InsertElementTxn ** aTxn);

  /** create a transaction for removing aElement from its parent.
    */
  NS_IMETHOD CreateTxnForDeleteElement(nsIDOMNode * aElement,
                                       DeleteElementTxn ** aTxn);


  NS_IMETHOD CreateTxnForDeleteSelection(EDirection aAction,
                                              EditAggregateTxn  ** aTxn);

  NS_IMETHOD CreateTxnForDeleteInsertionPoint(nsIDOMRange         *aRange, 
                                              EDirection aAction, 
                                              EditAggregateTxn    *aTxn);


  /** create a transaction for inserting aStringToInsert into aTextNode
    * if aTextNode is null, the string is inserted at the current selection.
    */
  NS_IMETHOD CreateTxnForInsertText(const nsAString & aStringToInsert,
                                    nsIDOMCharacterData *aTextNode,
                                    PRInt32 aOffset,
                                    InsertTextTxn ** aTxn);

  NS_IMETHOD CreateTxnForIMEText(const nsAString & aStringToInsert,
                                 IMETextTxn ** aTxn);

  /** create a transaction for adding a style sheet
    */
  NS_IMETHOD CreateTxnForAddStyleSheet(nsICSSStyleSheet* aSheet, AddStyleSheetTxn* *aTxn);

  /** create a transaction for removing a style sheet
    */
  NS_IMETHOD CreateTxnForRemoveStyleSheet(nsICSSStyleSheet* aSheet, RemoveStyleSheetTxn* *aTxn);
  
  NS_IMETHOD DeleteText(nsIDOMCharacterData *aElement,
                        PRUint32             aOffset,
                        PRUint32             aLength);

//  NS_IMETHOD DeleteRange(nsIDOMRange *aRange);

  NS_IMETHOD CreateTxnForDeleteText(nsIDOMCharacterData *aElement,
                                    PRUint32             aOffset,
                                    PRUint32             aLength,
                                    DeleteTextTxn      **aTxn);

  nsresult CreateTxnForDeleteCharacter(nsIDOMCharacterData  *aData,
                                       PRUint32              aOffset,
                                       nsIEditor::EDirection aDirection,
                                       DeleteTextTxn       **aTxn);
	
  NS_IMETHOD CreateTxnForSplitNode(nsIDOMNode *aNode,
                                   PRUint32    aOffset,
                                   SplitElementTxn **aTxn);

  NS_IMETHOD CreateTxnForJoinNode(nsIDOMNode  *aLeftNode,
                                  nsIDOMNode  *aRightNode,
                                  JoinElementTxn **aTxn);

  NS_IMETHOD DeleteSelectionAndPrepareToCreateNode(nsCOMPtr<nsIDOMNode> &parentSelectedNode, 
                                                   PRInt32& offsetOfNewNode);

  // called after a transaction is done successfully
  NS_IMETHOD DoAfterDoTransaction(nsITransaction *aTxn);
  // called after a transaction is undone successfully
  NS_IMETHOD DoAfterUndoTransaction();
  // called after a transaction is redone successfully
  NS_IMETHOD DoAfterRedoTransaction();

  typedef enum {
    eDocumentCreated,
    eDocumentToBeDestroyed,
    eDocumentStateChanged
  } TDocumentListenerNotification;
  
  // tell the doc state listeners that the doc state has changed
  NS_IMETHOD NotifyDocumentListeners(TDocumentListenerNotification aNotificationType);
  
  /** make the given selection span the entire document */
  NS_IMETHOD SelectEntireDocument(nsISelection *aSelection);

  /* Helper for output routines -- we expect subclasses to override this */
  NS_IMETHOD GetWrapWidth(PRInt32* aWrapCol);

  /** helper method for scrolling the selection into view after
   *  an edit operation. aScrollToAnchor should be PR_TRUE if you
   *  want to scroll to the point where the selection was started.
   *  If PR_FALSE, it attempts to scroll the end of the selection into view.
   *
   *  Editor methods *should* call this method instead of the versions
   *  in the various selection interfaces, since this version makes sure
   *  that the editor's sync/async settings for reflowing, painting, and
   *  scrolling match.
   */
  NS_IMETHOD ScrollSelectionIntoView(PRBool aScrollToAnchor);

  // stub.  see comment in source.                     
  virtual PRBool IsBlockNode(nsIDOMNode *aNode);
  
  // helper for GetPriorNode
  nsresult GetPriorNodeImpl(nsIDOMNode  *aCurrentNode, 
                            PRBool       aEditableNode,
                            nsCOMPtr<nsIDOMNode> *aResultNode,
                            PRBool       bNoBlockCrossing = PR_FALSE);

  // helper for GetNextNode
  nsresult GetNextNodeImpl(nsIDOMNode  *aCurrentNode, 
                           PRBool       aEditableNode,
                           nsCOMPtr<nsIDOMNode> *aResultNode,
                           PRBool       bNoBlockCrossing = PR_FALSE);

  // Get nsIKBStateControl interface
  nsresult GetKBStateControl(nsIKBStateControl **aKBSC);


  // install the event listeners for the editor 
  nsresult InstallEventListeners();

  virtual nsresult CreateEventListeners() = 0;

  // unregister and release our event listeners
  virtual void RemoveEventListeners();

public:

  /** All editor operations which alter the doc should be prefaced
   *  with a call to StartOperation, naming the action and direction */
  NS_IMETHOD StartOperation(PRInt32 opID, nsIEditor::EDirection aDirection);

  /** All editor operations which alter the doc should be followed
   *  with a call to EndOperation */
  NS_IMETHOD EndOperation();

  /** routines for managing the preservation of selection across 
   *  various editor actions */
  PRBool   ArePreservingSelection();
  nsresult PreserveSelectionAcrossActions(nsISelection *aSel);
  nsresult RestorePreservedSelection(nsISelection *aSel);
  void     StopPreservingSelection();


  /** return the string that represents text nodes in the content tree */
  static nsresult GetTextNodeTag(nsAString& aOutString);

  /** 
   * SplitNode() creates a new node identical to an existing node, and split the contents between the two nodes
   * @param aExistingRightNode   the node to split.  It will become the new node's next sibling.
   * @param aOffset              the offset of aExistingRightNode's content|children to do the split at
   * @param aNewLeftNode         [OUT] the new node resulting from the split, becomes aExistingRightNode's previous sibling.
   * @param aParent              the parent of aExistingRightNode
   */
  nsresult SplitNodeImpl(nsIDOMNode *aExistingRightNode,
                         PRInt32     aOffset,
                         nsIDOMNode *aNewLeftNode,
                         nsIDOMNode *aParent);

  /** 
   * JoinNodes() takes 2 nodes and merge their content|children.
   * @param aNodeToKeep   The node that will remain after the join.
   * @param aNodeToJoin   The node that will be joined with aNodeToKeep.
   *                      There is no requirement that the two nodes be of the same type.
   * @param aParent       The parent of aNodeToKeep
   * @param aNodeToKeepIsFirst  if PR_TRUE, the contents|children of aNodeToKeep come before the
   *                            contents|children of aNodeToJoin, otherwise their positions are switched.
   */
  nsresult JoinNodesImpl(nsIDOMNode *aNodeToKeep,
                         nsIDOMNode *aNodeToJoin,
                         nsIDOMNode *aParent,
                         PRBool      aNodeToKeepIsFirst);

  /**
   *  Set aOffset to the offset of aChild in aParent.  
   *  Returns an error if aChild is not an immediate child of aParent.
   */
  static nsresult GetChildOffset(nsIDOMNode *aChild, 
                                 nsIDOMNode *aParent, 
                                 PRInt32    &aOffset);

  /**
   *  Set aParent to the parent of aChild.
   *  Set aOffset to the offset of aChild in aParent.  
   */
  static nsresult GetNodeLocation(nsIDOMNode *aChild, 
                                 nsCOMPtr<nsIDOMNode> *aParent, 
                                 PRInt32    *aOffset);

  /** returns the number of things inside aNode in the out-param aCount.  
    * @param  aNode is the node to get the length of.  
    *         If aNode is text, returns number of characters. 
    *         If not, returns number of children nodes.
    * @param  aCount [OUT] the result of the above calculation.
    */
  static nsresult GetLengthOfDOMNode(nsIDOMNode *aNode, PRUint32 &aCount);

  /** get the node immediately prior to aCurrentNode
    * @param aCurrentNode   the node from which we start the search
    * @param aEditableNode  if PR_TRUE, only return an editable node
    * @param aResultNode    [OUT] the node that occurs before aCurrentNode in the tree,
    *                       skipping non-editable nodes if aEditableNode is PR_TRUE.
    *                       If there is no prior node, aResultNode will be nsnull.
    */
  nsresult GetPriorNode(nsIDOMNode  *aCurrentNode, 
                        PRBool       aEditableNode,
                        nsCOMPtr<nsIDOMNode> *aResultNode,
                        PRBool       bNoBlockCrossing = PR_FALSE);

  // and another version that takes a {parent,offset} pair rather than a node
  nsresult GetPriorNode(nsIDOMNode  *aParentNode, 
                        PRInt32      aOffset, 
                        PRBool       aEditableNode, 
                        nsCOMPtr<nsIDOMNode> *aResultNode,
                        PRBool       bNoBlockCrossing = PR_FALSE);
                       
  /** get the node immediately after to aCurrentNode
    * @param aCurrentNode   the node from which we start the search
    * @param aEditableNode  if PR_TRUE, only return an editable node
    * @param aResultNode    [OUT] the node that occurs after aCurrentNode in the tree,
    *                       skipping non-editable nodes if aEditableNode is PR_TRUE.
    *                       If there is no prior node, aResultNode will be nsnull.
    */
  nsresult GetNextNode(nsIDOMNode  *aCurrentNode, 
                       PRBool       aEditableNode,
                       nsCOMPtr<nsIDOMNode> *aResultNode,
                       PRBool       bNoBlockCrossing = PR_FALSE);

  // and another version that takes a {parent,offset} pair rather than a node
  nsresult GetNextNode(nsIDOMNode  *aParentNode, 
                       PRInt32      aOffset, 
                       PRBool       aEditableNode, 
                       nsCOMPtr<nsIDOMNode> *aResultNode,
                       PRBool       bNoBlockCrossing = PR_FALSE);

  /** Get the rightmost child of aCurrentNode;
    * return nsnull if aCurrentNode has no children.
    */
  nsCOMPtr<nsIDOMNode> GetRightmostChild(nsIDOMNode *aCurrentNode, 
                                         PRBool      bNoBlockCrossing = PR_FALSE);

  /** Get the leftmost child of aCurrentNode;
    * return nsnull if aCurrentNode has no children.
    */
  nsCOMPtr<nsIDOMNode> GetLeftmostChild(nsIDOMNode  *aCurrentNode, 
                                         PRBool      bNoBlockCrossing = PR_FALSE);

  /** returns PR_TRUE if aNode is of the type implied by aTag */
  static inline PRBool NodeIsType(nsIDOMNode *aNode, nsIAtom *aTag)
  {
    return GetTag(aNode) == aTag;
  }

  // we should get rid of this method if we can
  static inline PRBool NodeIsTypeString(nsIDOMNode *aNode, const nsAString &aTag)
  {
    nsIAtom *nodeAtom = GetTag(aNode);
    return nodeAtom && nodeAtom->Equals(aTag);
  }


  /** returns PR_TRUE if aParent can contain a child of type aTag */
  PRBool CanContainTag(nsIDOMNode* aParent, const nsAString &aTag);
  PRBool TagCanContain(const nsAString &aParentTag, nsIDOMNode* aChild);
  virtual PRBool TagCanContainTag(const nsAString &aParentTag, const nsAString &aChildTag);

  /** returns PR_TRUE if aNode is our root node */
  PRBool IsRootNode(nsIDOMNode *inNode);

  /** returns PR_TRUE if aNode is a descendant of our root node */
  PRBool IsDescendantOfBody(nsIDOMNode *inNode);

  /** returns PR_TRUE if aNode is a container */
  virtual PRBool IsContainer(nsIDOMNode *aNode);

  /** returns PR_TRUE if aNode is an editable node */
  PRBool IsEditable(nsIDOMNode *aNode);

  virtual PRBool IsTextInDirtyFrameVisible(nsIDOMNode *aNode);

  /** returns PR_TRUE if aNode is a MozEditorBogus node */
  PRBool IsMozEditorBogusNode(nsIDOMNode *aNode);

  /** returns PR_TRUE if content is an merely formatting whitespacce */
  PRBool IsEmptyTextContent(nsIContent* aContent);

  /** counts number of editable child nodes */
  nsresult CountEditableChildren(nsIDOMNode *aNode, PRUint32 &outCount);
  
  /** Find the deep first and last children. Returned nodes are AddReffed */
  nsresult GetFirstEditableNode(nsIDOMNode *aRoot, nsCOMPtr<nsIDOMNode> *outFirstNode);
#ifdef XXX_DEAD_CODE
  nsresult GetLastEditableNode(nsIDOMNode *aRoot, nsCOMPtr<nsIDOMNode> *outLastNode);
#endif

  nsresult GetIMEBufferLength(PRInt32* length);
  PRBool   IsIMEComposing();    /* test if IME is in composition state */
  void     SetIsIMEComposing(); /* call this before |IsIMEComposing()| */

  /** from html rules code - migration in progress */
  static nsresult GetTagString(nsIDOMNode *aNode, nsAString& outString);
  static nsIAtom *GetTag(nsIDOMNode *aNode);
  virtual PRBool NodesSameType(nsIDOMNode *aNode1, nsIDOMNode *aNode2);
  static PRBool IsTextOrElementNode(nsIDOMNode *aNode);
  static PRBool IsTextNode(nsIDOMNode *aNode);
  
  static PRInt32 GetIndexOf(nsIDOMNode *aParent, nsIDOMNode *aChild);
  static nsCOMPtr<nsIDOMNode> GetChildAt(nsIDOMNode *aParent, PRInt32 aOffset);
  
  static nsresult GetStartNodeAndOffset(nsISelection *aSelection, nsCOMPtr<nsIDOMNode> *outStartNode, PRInt32 *outStartOffset);
  static nsresult GetEndNodeAndOffset(nsISelection *aSelection, nsCOMPtr<nsIDOMNode> *outEndNode, PRInt32 *outEndOffset);
#if DEBUG_JOE
  static void DumpNode(nsIDOMNode *aNode, PRInt32 indent=0);
#endif

  // Helpers to add a node to the selection. 
  // Used by table cell selection methods
  nsresult CreateRange(nsIDOMNode *aStartParent, PRInt32 aStartOffset,
                       nsIDOMNode *aEndParent, PRInt32 aEndOffset,
                       nsIDOMRange **aRange);

  // Creates a range with just the supplied node and appends that to the selection
  nsresult AppendNodeToSelectionAsRange(nsIDOMNode *aNode);
  // When you are using AppendNodeToSelectionAsRange, call this first to start a new selection
  nsresult ClearSelection();

  nsresult IsPreformatted(nsIDOMNode *aNode, PRBool *aResult);

  nsresult SplitNodeDeep(nsIDOMNode *aNode, 
                         nsIDOMNode *aSplitPointParent, 
                         PRInt32 aSplitPointOffset,
                         PRInt32 *outOffset,
                         PRBool  aNoEmptyContainers = PR_FALSE,
                         nsCOMPtr<nsIDOMNode> *outLeftNode = 0,
                         nsCOMPtr<nsIDOMNode> *outRightNode = 0);
  nsresult JoinNodeDeep(nsIDOMNode *aLeftNode, nsIDOMNode *aRightNode, nsCOMPtr<nsIDOMNode> *aOutJoinNode, PRInt32 *outOffset); 

  nsresult GetString(const nsAString& name, nsAString& value);

  nsresult BeginUpdateViewBatch(void);
  virtual nsresult EndUpdateViewBatch(void);

  PRBool GetShouldTxnSetSelection();

  nsresult HandleInlineSpellCheck(PRInt32 action,
                                    nsISelection *aSelection,
                                    nsIDOMNode *previousSelectedNode,
                                    PRInt32 previousSelectedOffset,
                                    nsIDOMNode *aStartNode,
                                    PRInt32 aStartOffset,
                                    nsIDOMNode *aEndNode,
                                    PRInt32 aEndOffset);

  already_AddRefed<nsIDOMEventReceiver> GetDOMEventReceiver();

  // Fast non-refcounting editor root element accessor
  nsIDOMElement *GetRoot();

public:
  // Argh!  These transaction names are used by PlaceholderTxn and
  // nsPlaintextEditor.  They should be localized to those classes.
  static nsIAtom *gTypingTxnName;
  static nsIAtom *gIMETxnName;
  static nsIAtom *gDeleteTxnName;

protected:

  PRUint32        mModCount;		// number of modifications (for undo/redo stack)
  PRUint32        mFlags;		// behavior flags. See nsIPlaintextEditor.idl for the flags we use.
  
  nsWeakPtr       mPresShellWeak;   // weak reference to the nsIPresShell
  nsWeakPtr       mSelConWeak;   // weak reference to the nsISelectionController
  nsIViewManager *mViewManager;
  PRInt32         mUpdateCount;
  nsCOMPtr<nsIInlineSpellChecker> mInlineSpellChecker;  // used for real-time spellchecking
  nsCOMPtr<nsITransactionManager> mTxnMgr;
  nsWeakPtr         mPlaceHolderTxn;     // weak reference to placeholder for begin/end batch purposes
  nsIAtom          *mPlaceHolderName;    // name of placeholder transaction
  PRInt32           mPlaceHolderBatch;   // nesting count for batching
  nsSelectionState *mSelState;           // saved selection state for placeholder txn batching
  nsSelectionState  mSavedSel;           // cached selection for nsAutoSelectionReset
  nsRangeUpdater    mRangeUpdater;       // utility class object for maintaining preserved ranges
  nsCOMPtr<nsIDOMElement> mRootElement;    // cached root node
  PRInt32           mAction;             // the current editor action
  EDirection        mDirection;          // the current direction of editor action
  
  // data necessary to build IME transactions
  nsIPrivateTextRangeList*      mIMETextRangeList; // IME special selection ranges
  nsCOMPtr<nsIDOMCharacterData> mIMETextNode;      // current IME text node
  PRUint32                      mIMETextOffset;    // offset in text node where IME comp string begins
  PRUint32                      mIMEBufferLength;  // current length of IME comp string
  PRPackedBool                  mInIMEMode;        // are we inside an IME composition?
  PRPackedBool                  mIsIMEComposing;   // is IME in composition state?
                                                   // This is different from mInIMEMode. see Bug 98434.

  PRPackedBool                  mShouldTxnSetSelection;  // turn off for conservative selection adjustment by txns
  // various listeners
  nsVoidArray*                  mActionListeners;  // listens to all low level actions on the doc
  nsVoidArray*                  mEditorObservers;   // just notify once per high level change
  nsCOMPtr<nsISupportsArray>    mDocStateListeners;// listen to overall doc state (dirty or not, just created, etc)

  PRInt8                        mDocDirtyState;		// -1 = not initialized
  nsWeakPtr        mDocWeak;  // weak reference to the nsIDOMDocument
  // The form field as an event receiver
  nsCOMPtr<nsIDOMEventReceiver> mDOMEventReceiver;

  nsString* mPhonetic;

  nsCOMPtr<nsIDOMEventListener> mKeyListenerP;
  nsCOMPtr<nsIDOMEventListener> mMouseListenerP;
  nsCOMPtr<nsIDOMEventListener> mTextListenerP;
  nsCOMPtr<nsIDOMEventListener> mCompositionListenerP;
  nsCOMPtr<nsIDOMEventListener> mDragListenerP;
  nsCOMPtr<nsIDOMEventListener> mFocusListenerP;

  friend PRBool NSCanUnload(nsISupports* serviceMgr);
  friend class nsAutoTxnsConserveSelection;
  friend class nsAutoSelectionReset;
  friend class nsAutoRules;
  friend class nsRangeUpdater;
};


#endif
