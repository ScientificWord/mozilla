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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Original Author: David W. Hyatt (hyatt@netscape.com)
 *   Alec Flett <alecf@netscape.com>
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

#ifndef nsBindingManager_h_
#define nsBindingManager_h_

#include "nsStubMutationObserver.h"
#include "pldhash.h"
#include "nsInterfaceHashtable.h"
#include "nsRefPtrHashtable.h"
#include "nsURIHashKey.h"
#include "nsCycleCollectionParticipant.h"
#include "nsXBLBinding.h"
#include "nsTArray.h"

class nsIContent;
class nsIXPConnectWrappedJS;
class nsIAtom;
class nsIDOMNodeList;
class nsIDocument;
class nsIURI;
class nsIXBLDocumentInfo;
class nsIStreamListener;
class nsStyleSet;
class nsXBLBinding;
template<class E> class nsRefPtr;
typedef nsTArray<nsRefPtr<nsXBLBinding> > nsBindingList;
template<class T> class nsRunnableMethod;
class nsIPrincipal;

class nsBindingManager : public nsStubMutationObserver
{
public:
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS

  NS_DECL_NSIMUTATIONOBSERVER_CONTENTAPPENDED
  NS_DECL_NSIMUTATIONOBSERVER_CONTENTINSERTED
  NS_DECL_NSIMUTATIONOBSERVER_CONTENTREMOVED

  nsBindingManager(nsIDocument* aDocument);
  ~nsBindingManager();

  nsXBLBinding* GetBinding(nsIContent* aContent);
  nsresult SetBinding(nsIContent* aContent, nsXBLBinding* aBinding);

  nsIContent* GetInsertionParent(nsIContent* aContent);
  nsresult SetInsertionParent(nsIContent* aContent, nsIContent* aResult);

  /**
   * Notify the binding manager that an element
   * has been moved from one document to another,
   * so that it can update any bindings or
   * nsIAnonymousContentCreator-created anonymous
   * content that may depend on the document.
   * @param aContent the element that's being moved
   * @param aOldDocument the old document in which the
   *   content resided. May be null if the the content
   *   was not in any document.
   * @param aNewDocument the document in which the
   *   content will reside. May be null if the content
   *   will not reside in any document, or if the
   *   content is being destroyed.
   */
  nsresult ChangeDocumentFor(nsIContent* aContent, nsIDocument* aOldDocument,
                             nsIDocument* aNewDocument);

  nsIAtom* ResolveTag(nsIContent* aContent, PRInt32* aNameSpaceID);

  /**
   * Return a list of all explicit children, including any children
   * that may have been inserted via XBL insertion points.
   */
  nsresult GetContentListFor(nsIContent* aContent, nsIDOMNodeList** aResult);

  /**
   * Set the insertion point children for the specified element.
   * The binding manager assumes ownership of aList.
   */
  nsresult SetContentListFor(nsIContent* aContent,
                             nsInsertionPointList* aList);

  /**
   * Determine whether or not the explicit child list has been altered
   * by XBL insertion points.
   */
  PRBool HasContentListFor(nsIContent* aContent);

  /**
   * For a given element, retrieve the anonymous child content.
   */
  nsresult GetAnonymousNodesFor(nsIContent* aContent, nsIDOMNodeList** aResult);

  /**
   * Set the anonymous child content for the specified element.
   * The binding manager assumes ownership of aList.
   */
  nsresult SetAnonymousNodesFor(nsIContent* aContent,
                                nsInsertionPointList* aList);

  /**
   * Retrieves the anonymous list of children if the element has one;
   * otherwise, retrieves the list of explicit children. N.B. that if
   * the explicit child list has not been altered by XBL insertion
   * points, then aResult will be null.
   */
  nsresult GetXBLChildNodesFor(nsIContent* aContent, nsIDOMNodeList** aResult);

  /**
   * Given a parent element and a child content, determine where the
   * child content should be inserted in the parent element's
   * anonymous content tree. Specifically, aChild should be inserted
   * beneath aResult at the index specified by aIndex.
   */
  nsIContent* GetInsertionPoint(nsIContent* aParent,
                                nsIContent* aChild, PRUint32* aIndex);

  /**
   * Return the unfiltered insertion point for the specified parent
   * element. If other filtered insertion points exist,
   * aMultipleInsertionPoints will be set to true.
   */
  nsIContent* GetSingleInsertionPoint(nsIContent* aParent, PRUint32* aIndex,
                                      PRBool* aMultipleInsertionPoints);

  nsresult AddLayeredBinding(nsIContent* aContent, nsIURI* aURL,
                             nsIPrincipal* aOriginPrincipal);
  nsresult RemoveLayeredBinding(nsIContent* aContent, nsIURI* aURL);
  nsresult LoadBindingDocument(nsIDocument* aBoundDoc, nsIURI* aURL,
                               nsIPrincipal* aOriginPrincipal);

  nsresult AddToAttachedQueue(nsXBLBinding* aBinding);
  void ProcessAttachedQueue(PRUint32 aSkipSize = 0);

  void ExecuteDetachedHandlers();

  nsresult PutXBLDocumentInfo(nsIXBLDocumentInfo* aDocumentInfo);
  nsIXBLDocumentInfo* GetXBLDocumentInfo(nsIURI* aURI);
  void RemoveXBLDocumentInfo(nsIXBLDocumentInfo* aDocumentInfo);

  nsresult PutLoadingDocListener(nsIURI* aURL, nsIStreamListener* aListener);
  nsIStreamListener* GetLoadingDocListener(nsIURI* aURL);
  void RemoveLoadingDocListener(nsIURI* aURL);

  void FlushSkinBindings();

  nsresult GetBindingImplementation(nsIContent* aContent, REFNSIID aIID, void** aResult);

  PRBool ShouldBuildChildFrames(nsIContent* aContent);

  // Style rule methods
  nsresult WalkRules(nsStyleSet* aStyleSet, 
                     nsIStyleRuleProcessor::EnumFunc aFunc,
                     RuleProcessorData* aData,
                     PRBool* aCutOffInheritance);

  NS_HIDDEN_(void) Traverse(nsIContent *aContent,
                            nsCycleCollectionTraversalCallback &cb);

  NS_DECL_CYCLE_COLLECTION_CLASS(nsBindingManager)

  // Notify the binding manager when an outermost update begins and
  // ends.  The end method can execute script.
  void BeginOutermostUpdate();
  void EndOutermostUpdate();

  // Called when the document is going away
  void DropDocumentReference();

protected:
  nsIXPConnectWrappedJS* GetWrappedJS(nsIContent* aContent);
  nsresult SetWrappedJS(nsIContent* aContent, nsIXPConnectWrappedJS* aResult);

  nsresult GetXBLChildNodesInternal(nsIContent* aContent,
                                    nsIDOMNodeList** aResult,
                                    PRBool* aIsAnonymousContentList);
  nsresult GetAnonymousNodesInternal(nsIContent* aContent,
                                     nsIDOMNodeList** aResult,
                                     PRBool* aIsAnonymousContentList);

  nsIContent* GetNestedInsertionPoint(nsIContent* aParent, nsIContent* aChild);
  nsIContent* GetNestedSingleInsertionPoint(nsIContent* aParent,
                                            PRBool* aMultipleInsertionPoints);

  // Called by ContentAppended and ContentInserted to handle a single child
  // insertion.  aChild must not be null.  aContainer may be null.
  // aIndexInContainer is the index of the child in the parent.  aAppend is
  // true if this child is being appended, not inserted.
  void HandleChildInsertion(nsIContent* aContainer, nsIContent* aChild,
                            PRUint32 aIndexInContainer, PRBool aAppend);

  // Same as ProcessAttachedQueue, but also nulls out
  // mProcessAttachedQueueEvent
  void DoProcessAttachedQueue();

  // Post an event to process the attached queue.
  void PostProcessAttachedQueueEvent();

// MEMBER VARIABLES
protected: 
  void RemoveInsertionParent(nsIContent* aParent);
  // A mapping from nsIContent* to the nsXBLBinding* that is
  // installed on that element.
  nsRefPtrHashtable<nsISupportsHashKey,nsXBLBinding> mBindingTable;

  // A mapping from nsIContent* to an nsIDOMNodeList*
  // (nsAnonymousContentList*).  This list contains an accurate
  // reflection of our *explicit* children (once intermingled with
  // insertion points) in the altered DOM.  There is an entry for a
  // content node in this table only if that content node has some
  // <children> kids.
  PLDHashTable mContentListTable;

  // A mapping from nsIContent* to an nsIDOMNodeList*
  // (nsAnonymousContentList*).  This list contains an accurate
  // reflection of our *anonymous* children (if and only if they are
  // intermingled with insertion points) in the altered DOM.  This
  // table is not used if no insertion points were defined directly
  // underneath a <content> tag in a binding.  The NodeList from the
  // <content> is used instead as a performance optimization.  There
  // is an entry for a content node in this table only if that content
  // node has a binding with a <content> attached and this <content>
  // contains <children> elements directly.
  PLDHashTable mAnonymousNodesTable;

  // A mapping from nsIContent* to nsIContent*.  The insertion parent
  // is our one true parent in the transformed DOM.  This gives us a
  // more-or-less O(1) way of obtaining our transformed parent.
  PLDHashTable mInsertionParentTable;

  // A mapping from nsIContent* to nsIXPWrappedJS* (an XPConnect
  // wrapper for JS objects).  For XBL bindings that implement XPIDL
  // interfaces, and that get referred to from C++, this table caches
  // the XPConnect wrapper for the binding.  By caching it, I control
  // its lifetime, and I prevent a re-wrap of the same script object
  // (in the case where multiple bindings in an XBL inheritance chain
  // both implement an XPIDL interface).
  PLDHashTable mWrapperTable;

  // A mapping from a URL (a string) to nsIXBLDocumentInfo*.  This table
  // is the cache of all binding documents that have been loaded by a
  // given bound document.
  nsInterfaceHashtable<nsURIHashKey,nsIXBLDocumentInfo> mDocumentTable;

  // A mapping from a URL (a string) to a nsIStreamListener. This
  // table is the currently loading binding docs.  If they're in this
  // table, they have not yet finished loading.
  nsInterfaceHashtable<nsURIHashKey,nsIStreamListener> mLoadingDocTable;

  // A queue of binding attached event handlers that are awaiting execution.
  nsBindingList mAttachedStack;
  PRPackedBool mProcessingAttachedStack;
  PRPackedBool mDestroyed;
  PRUint32 mAttachedStackSizeOnOutermost;

  // Our posted event to process the attached queue, if any
  friend class nsRunnableMethod<nsBindingManager>;
  nsCOMPtr<nsIRunnable> mProcessAttachedQueueEvent;

  // Our document.  This is a weak ref; the document owns us
  nsIDocument* mDocument; 
};

#endif
