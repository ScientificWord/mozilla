//* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is Mozilla History System
 *
 * The Initial Developer of the Original Code is
 * Google Inc.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Brett Wilson <brettw@gmail.com> (original author)
 *   Dietrich Ayala <dietrich@mozilla.com>
 *   Asaf Romano <mano@mozilla.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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


#include <stdio.h>
#include "nsNavHistory.h"
#include "nsNavBookmarks.h"

#include "nsIArray.h"
#include "nsCOMPtr.h"
#include "nsDebug.h"
#include "nsFaviconService.h"
#include "nsHashPropertyBag.h"
#include "nsIComponentManager.h"
#include "nsIDateTimeFormat.h"
#include "nsIDOMElement.h"
#include "nsILocale.h"
#include "nsILocaleService.h"
#include "nsILocalFile.h"
#include "nsIDynamicContainer.h"
#include "nsIServiceManager.h"
#include "nsISupportsPrimitives.h"
#ifdef MOZ_XUL
#include "nsITreeColumns.h"
#endif
#include "nsIURI.h"
#include "nsIURL.h"
#include "nsIWritablePropertyBag.h"
#include "nsITaggingService.h"
#include "nsNetUtil.h"
#include "nsPrintfCString.h"
#include "nsPromiseFlatString.h"
#include "nsString.h"
#include "nsUnicharUtils.h"
#include "prtime.h"
#include "prprf.h"
#include "mozStorageHelper.h"
#include "nsAnnotationService.h"
#include "nsCycleCollectionParticipant.h"

// What we want is: NS_INTERFACE_MAP_ENTRY(self) for static IID accessors,
// but some of our classes (like nsNavHistoryResult) have an ambiguous base
// class of nsISupports which prevents this from working (the default macro
// converts it to nsISupports, then addrefs it, then returns it). Therefore, we
// expand the macro here and change it so that it works. Yuck.
#define NS_INTERFACE_MAP_STATIC_AMBIGUOUS(_class) \
  if (aIID.Equals(NS_GET_IID(_class))) { \
    NS_ADDREF(this); \
    *aInstancePtr = this; \
    return NS_OK; \
  } else

// emulate string comparison (used for sorting) for PRTime and int
inline PRInt32 ComparePRTime(PRTime a, PRTime b)
{
  if (LL_CMP(a, <, b))
    return -1;
  else if (LL_CMP(a, >, b))
    return 1;
  return 0;
}
inline PRInt32 CompareIntegers(PRUint32 a, PRUint32 b)
{
  return a - b;
}


// nsNavHistoryResultNode ******************************************************

NS_IMPL_CYCLE_COLLECTION_0(nsNavHistoryResultNode)

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(nsNavHistoryResultNode)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsINavHistoryResultNode)
  NS_INTERFACE_MAP_ENTRY(nsINavHistoryResultNode)
NS_INTERFACE_MAP_END

NS_IMPL_CYCLE_COLLECTING_ADDREF_AMBIGUOUS(nsNavHistoryResultNode, nsINavHistoryResultNode)
NS_IMPL_CYCLE_COLLECTING_RELEASE_AMBIGUOUS(nsNavHistoryResultNode, nsINavHistoryResultNode)

nsNavHistoryResultNode::nsNavHistoryResultNode(
    const nsACString& aURI, const nsACString& aTitle, PRUint32 aAccessCount,
    PRTime aTime, const nsACString& aIconURI) :
  mParent(nsnull),
  mURI(aURI),
  mTitle(aTitle),
  mAccessCount(aAccessCount),
  mTime(aTime),
  mFaviconURI(aIconURI),
  mBookmarkIndex(-1),
  mItemId(-1),
  mDateAdded(0),
  mLastModified(0),
  mIndentLevel(-1),
  mViewIndex(-1)
{
  mTags.SetIsVoid(PR_TRUE);
}

NS_IMETHODIMP
nsNavHistoryResultNode::GetIcon(nsIURI** aURI)
{
  nsFaviconService* faviconService = nsFaviconService::GetFaviconService();
  NS_ENSURE_TRUE(faviconService, NS_ERROR_OUT_OF_MEMORY);
  if (mFaviconURI.IsEmpty()) {
    *aURI = nsnull;
    return NS_OK;
  }
  return faviconService->GetFaviconLinkForIconString(mFaviconURI, aURI);
}

NS_IMETHODIMP
nsNavHistoryResultNode::GetParent(nsINavHistoryContainerResultNode** aParent)
{
  NS_IF_ADDREF(*aParent = mParent);
  return NS_OK;
}

NS_IMETHODIMP
nsNavHistoryResultNode::GetParentResult(nsINavHistoryResult** aResult)
{
  *aResult = nsnull;
  if (IsContainer() && GetAsContainer()->mResult) {
    NS_ADDREF(*aResult = GetAsContainer()->mResult);
  } else if (mParent && mParent->mResult) {
    NS_ADDREF(*aResult = mParent->mResult);
  } else {
   return NS_ERROR_UNEXPECTED;
  }

  return NS_OK;
}

NS_IMETHODIMP
nsNavHistoryResultNode::GetTags(nsAString& aTags) {
  // Only URI-nodes may be associated with tags
  if (!IsURI()) {
    aTags.Truncate();
    return NS_OK;
  }

  // Initially, the tags string is set to a void string (see constructor). We
  // then build it the first time this method called is called (and by that,
  // implicitly unset the void flag). Result observers may re-set the void flag
  // in order to force rebuilding of the tags string.
  if (!mTags.IsVoid()) {
    aTags.Assign(mTags);
    return NS_OK;
  }

  nsresult rv;
  nsCOMPtr<nsITaggingService> svc =
    do_GetService("@mozilla.org/browser/tagging-service;1", &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIURI> uri;
  rv = NS_NewURI(getter_AddRefs(uri), mURI);
  NS_ENSURE_SUCCESS(rv, rv);

  // build the tags string
  PRUnichar **tags;
  PRUint32 count;
  rv = svc->GetTagsForURI(uri, &count, &tags);
  NS_ENSURE_SUCCESS(rv, rv);
  if (count > 0) {
    for (PRUint32 i=0; i < count; i++) {
      mTags.Append(tags[i]);
      if (i < count -1) { // separate with commas
        mTags.Append(NS_LITERAL_STRING(", "));
      }
    }
    NS_FREE_XPCOM_ALLOCATED_POINTER_ARRAY(count, tags);
  }
  aTags.Assign(mTags);

  // If this node is a child of a history, we need to make sure
  // bookmarks-liveupdate is turned on for this query
  if (mParent && mParent->IsQuery()) {
    nsNavHistoryQueryResultNode* query = mParent->GetAsQuery();
    if (query->mLiveUpdate != QUERYUPDATE_COMPLEX_WITH_BOOKMARKS) {
      query->mLiveUpdate = QUERYUPDATE_COMPLEX_WITH_BOOKMARKS;
      nsNavHistoryResult* result = query->GetResult();
      NS_ENSURE_TRUE(result, NS_ERROR_FAILURE);
      result->AddAllBookmarksObserver(query);
    }
  }
  return NS_OK;
}

NS_IMETHODIMP
nsNavHistoryResultNode::GetPropertyBag(nsIWritablePropertyBag** aBag)
{
  nsNavHistoryResult* result = GetResult();
  NS_ENSURE_TRUE(result, NS_ERROR_FAILURE);
  return result->PropertyBagFor(this, aBag);
}


// nsNavHistoryResultNode::OnRemoving
//
//    This will zero out some values in case somebody still holds a reference

void
nsNavHistoryResultNode::OnRemoving()
{
  mParent = nsnull;
  mViewIndex = -1;
}


// nsNavHistoryResultNode::GetResult
//
//    This will find the result for this node. We can ask the nearest container
//    for this value (either ourselves or our parents should be a container,
//    and all containers have result pointers).

nsNavHistoryResult*
nsNavHistoryResultNode::GetResult()
{
  nsNavHistoryResultNode* node = this;
  do {
    if (node->IsContainer()) {
      nsNavHistoryContainerResultNode* container =
        static_cast<nsNavHistoryContainerResultNode*>(node);
      NS_ASSERTION(container->mResult, "Containers must have valid results");
      return container->mResult;
    }
    node = node->mParent;
  } while (node);
  NS_NOTREACHED("No container node found in hierarchy!");
  return nsnull;
}

// nsNavHistoryResultNode::GetGeneratingOptions
//
//    Searches up the tree for the closest node that has an options structure.
//    This will tell us the options that were used to generate this node.
//
//    Be careful, this function walks up the tree, so it can not be used when
//    result nodes are created because they have no parent. Only call this
//    function after the tree has been built.

nsNavHistoryQueryOptions*
nsNavHistoryResultNode::GetGeneratingOptions()
{
  if (! mParent) {
    // When we have no parent, it either means we haven't built the tree yet,
    // in which case calling this function is a bug, or this node is the root
    // of the tree. When we are the root of the tree, our own options are the
    // generating options.
    if (IsContainer())
      return GetAsContainer()->mOptions;
    NS_NOTREACHED("Can't find a generating node for this container, perhaps FillStats has not been called on this tree yet?");
    return nsnull;
  }

  nsNavHistoryContainerResultNode* cur = mParent;
  while (cur) {
    if (cur->IsFolder())
      return cur->GetAsFolder()->mOptions;
    else if (cur->IsQuery())
      return cur->GetAsQuery()->mOptions;
    cur = cur->mParent;
  }
  // we should always find a folder or query node as an ancestor
  NS_NOTREACHED("Can't find a generating node for this container, the tree seemes corrupted.");
  return nsnull;
}


// nsNavHistoryVisitResultNode *************************************************

NS_IMPL_ISUPPORTS_INHERITED1(nsNavHistoryVisitResultNode,
                             nsNavHistoryResultNode,
                             nsINavHistoryVisitResultNode)

nsNavHistoryVisitResultNode::nsNavHistoryVisitResultNode(
    const nsACString& aURI, const nsACString& aTitle, PRUint32 aAccessCount,
    PRTime aTime, const nsACString& aIconURI, PRInt64 aSession) :
  nsNavHistoryResultNode(aURI, aTitle, aAccessCount, aTime, aIconURI),
  mSessionId(aSession)
{
}


// nsNavHistoryFullVisitResultNode *********************************************

NS_IMPL_ISUPPORTS_INHERITED1(nsNavHistoryFullVisitResultNode,
                             nsNavHistoryVisitResultNode,
                             nsINavHistoryFullVisitResultNode)

nsNavHistoryFullVisitResultNode::nsNavHistoryFullVisitResultNode(
    const nsACString& aURI, const nsACString& aTitle, PRUint32 aAccessCount,
    PRTime aTime, const nsACString& aIconURI, PRInt64 aSession,
    PRInt64 aVisitId, PRInt64 aReferringVisitId, PRInt32 aTransitionType) :
  nsNavHistoryVisitResultNode(aURI, aTitle, aAccessCount, aTime, aIconURI,
                              aSession),
  mVisitId(aVisitId),
  mReferringVisitId(aReferringVisitId),
  mTransitionType(aTransitionType)
{
}

// nsNavHistoryContainerResultNode *********************************************

NS_IMPL_CYCLE_COLLECTION_CLASS(nsNavHistoryContainerResultNode)

NS_IMPL_CYCLE_COLLECTION_UNLINK_BEGIN_INHERITED(nsNavHistoryContainerResultNode, nsNavHistoryResultNode)
  NS_IMPL_CYCLE_COLLECTION_UNLINK_NSCOMPTR(mResult)
  NS_IMPL_CYCLE_COLLECTION_UNLINK_NSCOMARRAY(mChildren)
NS_IMPL_CYCLE_COLLECTION_UNLINK_END 

NS_IMPL_CYCLE_COLLECTION_TRAVERSE_BEGIN_INHERITED(nsNavHistoryContainerResultNode, nsNavHistoryResultNode)
  NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSCOMPTR_AMBIGUOUS(mResult, nsINavHistoryResult)
  NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSCOMARRAY(mChildren)
NS_IMPL_CYCLE_COLLECTION_TRAVERSE_END

NS_IMPL_ADDREF_INHERITED(nsNavHistoryContainerResultNode, nsNavHistoryResultNode)
NS_IMPL_RELEASE_INHERITED(nsNavHistoryContainerResultNode, nsNavHistoryResultNode)

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION_INHERITED(nsNavHistoryContainerResultNode)
  NS_INTERFACE_MAP_STATIC_AMBIGUOUS(nsNavHistoryContainerResultNode)
  NS_INTERFACE_MAP_ENTRY(nsINavHistoryContainerResultNode)
NS_INTERFACE_MAP_END_INHERITING(nsNavHistoryResultNode)

nsNavHistoryContainerResultNode::nsNavHistoryContainerResultNode(
    const nsACString& aURI, const nsACString& aTitle,
    const nsACString& aIconURI, PRUint32 aContainerType, PRBool aReadOnly,
    const nsACString& aDynamicContainerType, nsNavHistoryQueryOptions* aOptions) :
  nsNavHistoryResultNode(aURI, aTitle, 0, 0, aIconURI),
  mResult(nsnull),
  mContainerType(aContainerType),
  mExpanded(PR_FALSE),
  mChildrenReadOnly(aReadOnly),
  mOptions(aOptions),
  mDynamicContainerType(aDynamicContainerType)
{
}

nsNavHistoryContainerResultNode::nsNavHistoryContainerResultNode(
    const nsACString& aURI, const nsACString& aTitle,
    PRTime aTime,
    const nsACString& aIconURI, PRUint32 aContainerType, PRBool aReadOnly,
    const nsACString& aDynamicContainerType, 
    nsNavHistoryQueryOptions* aOptions) :
  nsNavHistoryResultNode(aURI, aTitle, 0, aTime, aIconURI),
  mResult(nsnull),
  mContainerType(aContainerType),
  mExpanded(PR_FALSE),
  mChildrenReadOnly(aReadOnly),
  mDynamicContainerType(aDynamicContainerType),
  mOptions(aOptions)
{
}

// nsNavHistoryContainerResultNode::OnRemoving
//
//    Containers should notify their children that they are being removed
//    when the container is being removed.

void
nsNavHistoryContainerResultNode::OnRemoving()
{
  nsNavHistoryResultNode::OnRemoving();
  for (PRInt32 i = 0; i < mChildren.Count(); i ++)
    mChildren[i]->OnRemoving();
  mChildren.Clear();
}


// nsNavHistoryContainerResultNode::AreChildrenVisible
//
//    Folders can't always depend on their mViewIndex value to determine if
//    their children are visible because they can be root nodes. Root nodes
//    are visible if a tree is attached to the result.

PRBool
nsNavHistoryContainerResultNode::AreChildrenVisible()
{
  // can't see children when we're invisible
  if (! mExpanded)
    return PR_FALSE;

  // easy case, the node itself is visible
  if (mViewIndex >= 0)
    return PR_TRUE;

  nsNavHistoryResult* result = GetResult();
  if (! result) {
    NS_NOTREACHED("Invalid result");
    return PR_FALSE;
  }
  if (result->mRootNode == this && result->mView)
    return PR_TRUE;
  return PR_FALSE;
}


// nsNavHistoryContainerResultNode::GetContainerOpen

NS_IMETHODIMP
nsNavHistoryContainerResultNode::GetContainerOpen(PRBool *aContainerOpen)
{
  *aContainerOpen = mExpanded;
  return NS_OK;
}


// nsNavHistoryContainerResultNode::SetContainerOpen

NS_IMETHODIMP
nsNavHistoryContainerResultNode::SetContainerOpen(PRBool aContainerOpen)
{
  if (mExpanded && ! aContainerOpen)
    CloseContainer();
  else if (! mExpanded && aContainerOpen)
    OpenContainer();
  return NS_OK;
}


// nsNavHistoryContainerResultNode::OpenContainer
//
//    This handles the generic container case. Other container types should
//    override this to do their own handling.

nsresult
nsNavHistoryContainerResultNode::OpenContainer()
{
  NS_ASSERTION(! mExpanded, "Container must be expanded to close it");
  mExpanded = PR_TRUE;

  if (IsDynamicContainer()) {
    // dynamic container API may want to fill us
    nsresult rv;
    nsCOMPtr<nsIDynamicContainer> svc = do_GetService(mDynamicContainerType.get(), &rv);
    if (NS_SUCCEEDED(rv)) {
      svc->OnContainerNodeOpening(this, GetGeneratingOptions());
    } else {
      NS_WARNING("Unable to get dynamic container for ");
      NS_WARNING(mDynamicContainerType.get());
    }
    PRInt32 oldAccessCount = mAccessCount;
    FillStats();
    ReverseUpdateStats(mAccessCount - oldAccessCount);
  }

  nsNavHistoryResult* result = GetResult();
  NS_ENSURE_TRUE(result, NS_ERROR_FAILURE);
  if (result->GetView())
    result->GetView()->ContainerOpened(this);
  return NS_OK;
}


// nsNavHistoryContainerResultNode::CloseContainer
//
//    Set aUpdateVisible to redraw the screen, this is the normal operation.
//    This is set to false for the recursive calls since the root container
//    that is being closed will handle recomputation of the visible elements
//    for its entire subtree.

nsresult
nsNavHistoryContainerResultNode::CloseContainer(PRBool aUpdateView)
{
  NS_ASSERTION(mExpanded, "Container must be expanded to close it");

  // recursively close all child containers
  for (PRInt32 i = 0; i < mChildren.Count(); i ++) {
    if (mChildren[i]->IsContainer() && mChildren[i]->GetAsContainer()->mExpanded)
      mChildren[i]->GetAsContainer()->CloseContainer(PR_FALSE);
  }

  mExpanded = PR_FALSE;

  nsresult rv;
  if (IsDynamicContainer()) {
    // notify dynamic containers that we are closing
    nsCOMPtr<nsIDynamicContainer> svc = do_GetService(mDynamicContainerType.get(), &rv);
    if (NS_SUCCEEDED(rv))
      svc->OnContainerNodeClosed(this);
  }

  if (aUpdateView) {
    nsNavHistoryResult* result = GetResult();
    NS_ENSURE_TRUE(result, NS_ERROR_FAILURE);
    if (result->GetView())
      result->GetView()->ContainerClosed(this);
  }
  return NS_OK;
}


// nsNavHistoryContainerResultNode::FillStats
//
//    This builds up tree statistics from the bottom up. Call with a container
//    and the indent level of that container. To init the full tree, call with
//    the root container. The default indent level is -1, which is appropriate
//    for the root level.
//
//    CALL THIS AFTER FILLING ANY CONTAINER to update the parent and result
//    node pointers, even if you don't care about visit counts and last visit
//    dates.

void
nsNavHistoryContainerResultNode::FillStats()
{
  PRUint32 accessCount = 0;
  PRTime newTime = 0;

  for (PRInt32 i = 0; i < mChildren.Count(); i ++) {
    nsNavHistoryResultNode* node = mChildren[i];
    node->mParent = this;
    node->mIndentLevel = mIndentLevel + 1;
    if (node->IsContainer()) {
      nsNavHistoryContainerResultNode* container = node->GetAsContainer();
      container->mResult = mResult;
      container->FillStats();
    }
    accessCount += node->mAccessCount;
    // this is how container nodes get sorted by date
    // The container gets the most recent time of the child nodes.
    if (node->mTime > newTime)
      newTime = node->mTime;
  }

  if (mExpanded) {
    mAccessCount = accessCount;
    if (!IsQuery() || newTime > mTime)
      mTime = newTime;
  }
}


// nsNavHistoryContainerResultNode::ReverseUpdateStats
//
//    This is used when one container changes to do a minimal update of the
//    tree structure. When something changes, you want to call FillStats if
//    necessary and update this container completely. Then call this function
//    which will walk up the tree and fill in the previous containers.
//
//    Note that you have to tell us by how much our access count changed. Our
//    access count should already be set to the new value; this is used to
//    change the parents without having to re-count all their children.
//
//    This does NOT update the last visit date downward. Therefore, if you are
//    deleting a node that has the most recent last visit date, the parents
//    will not get their last visit dates downshifted accordingly. This is a
//    rather unusual case: we don't often delete things, and we usually don't
//    even show the last visit date for folders. Updating would be slower
//    because we would have to recompute it from scratch.

void
nsNavHistoryContainerResultNode::ReverseUpdateStats(PRInt32 aAccessCountChange)
{
  if (mParent) {
    mParent->mAccessCount += aAccessCountChange;
    PRBool timeChanged = PR_FALSE;
    if (mTime > mParent->mTime) {
      timeChanged = PR_TRUE;
      mParent->mTime = mTime;
    }

    // check sorting, the stats may have caused this node to move if the
    // sorting depended on something we are changing.
    PRUint16 sortMode = mParent->GetSortType();
    PRBool resorted = PR_FALSE;
    if (((sortMode == nsINavHistoryQueryOptions::SORT_BY_VISITCOUNT_ASCENDING ||
          sortMode == nsINavHistoryQueryOptions::SORT_BY_VISITCOUNT_DESCENDING) &&
         aAccessCountChange != 0) ||
        ((sortMode == nsINavHistoryQueryOptions::SORT_BY_DATE_ASCENDING ||
          sortMode == nsINavHistoryQueryOptions::SORT_BY_DATE_DESCENDING) &&
         timeChanged)) {

      PRUint32 ourIndex = mParent->FindChild(this);
      resorted = EnsureItemPosition(ourIndex);
    }
    if (!resorted) {
      // repaint visible rows
      nsNavHistoryResult* result = GetResult();
      if (result && result->GetView() && mParent->AreChildrenVisible()) {
        result->GetView()->ItemChanged(static_cast<nsINavHistoryContainerResultNode*>(mParent));
      }
    }

    mParent->ReverseUpdateStats(aAccessCountChange);
  }
}


// nsNavHistoryContainerResultNode::GetSortType
//
//    This walks up the tree until we find a query result node or the root to
//    get the sorting type.
//
//    See nsNavHistoryQueryResultNode::GetSortType

PRUint16
nsNavHistoryContainerResultNode::GetSortType()
{
  if (mParent)
    return mParent->GetSortType();
  else if (mResult)
    return mResult->mSortingMode;
  NS_NOTREACHED("We should always have a result");
  return nsINavHistoryQueryOptions::SORT_BY_NONE;
}

void
nsNavHistoryContainerResultNode::GetSortingAnnotation(nsACString& aAnnotation)
{
  if (mParent)
    mParent->GetSortingAnnotation(aAnnotation);
  else if (mResult)
    aAnnotation.Assign(mResult->mSortingAnnotation);
  else
    NS_NOTREACHED("We should always have a result");
}

// nsNavHistoryContainerResultNode::GetSortingComparator
//
//    Returns the sorting comparator function for the give sort type.
//    RETURNS NULL if there is no comparator.

nsNavHistoryContainerResultNode::SortComparator
nsNavHistoryContainerResultNode::GetSortingComparator(PRUint16 aSortType)
{
  switch (aSortType)
  {
    case nsINavHistoryQueryOptions::SORT_BY_NONE:
      return &SortComparison_Bookmark;
    case nsINavHistoryQueryOptions::SORT_BY_TITLE_ASCENDING:
      return &SortComparison_TitleLess;
    case nsINavHistoryQueryOptions::SORT_BY_TITLE_DESCENDING:
      return &SortComparison_TitleGreater;
    case nsINavHistoryQueryOptions::SORT_BY_DATE_ASCENDING:
      return &SortComparison_DateLess;
    case nsINavHistoryQueryOptions::SORT_BY_DATE_DESCENDING:
      return &SortComparison_DateGreater;
    case nsINavHistoryQueryOptions::SORT_BY_URI_ASCENDING:
      return &SortComparison_URILess;
    case nsINavHistoryQueryOptions::SORT_BY_URI_DESCENDING:
      return &SortComparison_URIGreater;
    case nsINavHistoryQueryOptions::SORT_BY_VISITCOUNT_ASCENDING:
      return &SortComparison_VisitCountLess;
    case nsINavHistoryQueryOptions::SORT_BY_VISITCOUNT_DESCENDING:
      return &SortComparison_VisitCountGreater;
    case nsINavHistoryQueryOptions::SORT_BY_KEYWORD_ASCENDING:
      return &SortComparison_KeywordLess;
    case nsINavHistoryQueryOptions::SORT_BY_KEYWORD_DESCENDING:
      return &SortComparison_KeywordGreater;
    case nsINavHistoryQueryOptions::SORT_BY_ANNOTATION_ASCENDING:
      return &SortComparison_AnnotationLess;
    case nsINavHistoryQueryOptions::SORT_BY_ANNOTATION_DESCENDING:
      return &SortComparison_AnnotationGreater;
    case nsINavHistoryQueryOptions::SORT_BY_DATEADDED_ASCENDING:
      return &SortComparison_DateAddedLess;
    case nsINavHistoryQueryOptions::SORT_BY_DATEADDED_DESCENDING:
      return &SortComparison_DateAddedGreater;
    case nsINavHistoryQueryOptions::SORT_BY_LASTMODIFIED_ASCENDING:
      return &SortComparison_LastModifiedLess;
    case nsINavHistoryQueryOptions::SORT_BY_LASTMODIFIED_DESCENDING:
      return &SortComparison_LastModifiedGreater;
    case nsINavHistoryQueryOptions::SORT_BY_TAGS_ASCENDING:
      return &SortComparison_TagsLess;
    case nsINavHistoryQueryOptions::SORT_BY_TAGS_DESCENDING:
      return &SortComparison_TagsGreater;
    default:
      NS_NOTREACHED("Bad sorting type");
      return nsnull;
  }
}


// nsNavHistoryContainerResultNode::RecursiveSort
//
//    This is used by Result::SetSortingMode and QueryResultNode::FillChildren to sort
//    the child list.
//
//    This does NOT update any visibility or tree information. The caller will
//    have to completely rebuild the visible list after this.

void
nsNavHistoryContainerResultNode::RecursiveSort(
    const char* aData, SortComparator aComparator)
{
  void* data = const_cast<void*>(static_cast<const void*>(aData));

  mChildren.Sort(aComparator, data);
  for (PRInt32 i = 0; i < mChildren.Count(); i ++) {
    if (mChildren[i]->IsContainer())
      mChildren[i]->GetAsContainer()->RecursiveSort(aData, aComparator);
  }
}


// nsNavHistoryContainerResultNode::FindInsertionPoint
//
//    This returns the index that the given item would fall on if it were to
//    be inserted using the given sorting.

PRUint32
nsNavHistoryContainerResultNode::FindInsertionPoint(
    nsNavHistoryResultNode* aNode, SortComparator aComparator,
    const char* aData, PRBool* aItemExists)
{
  if (aItemExists)
    (*aItemExists) = PR_FALSE;

  if (mChildren.Count() == 0)
    return 0;

  void* data = const_cast<void*>(static_cast<const void*>(aData));

  // The common case is the beginning or the end because this is used to insert
  // new items that are added to history, which is usually sorted by date.
  PRInt32 res;
  res = aComparator(aNode, mChildren[0], data);
  if (res <= 0) {
    if (aItemExists && res == 0)
      (*aItemExists) = PR_TRUE;
    return 0;
  }
  res = aComparator(aNode, mChildren[mChildren.Count() - 1], data);
  if (res >= 0) {
    if (aItemExists && res == 0)
      (*aItemExists) = PR_TRUE;
    return mChildren.Count();
  }

  PRUint32 beginRange = 0; // inclusive
  PRUint32 endRange = mChildren.Count(); // exclusive
  while (1) {
    if (beginRange == endRange)
      return endRange;
    PRUint32 center = beginRange + (endRange - beginRange) / 2;
    PRInt32 res = aComparator(aNode, mChildren[center], data);
    if (res <= 0) {
      endRange = center; // left side
      if (aItemExists && res == 0)
        (*aItemExists) = PR_TRUE;
    }
    else {
      beginRange = center + 1; // right site
    }
  }
}


// nsNavHistoryContainerResultNode::DoesChildNeedResorting
//
//    This checks the child node at the given index to see if its sorting is
//    correct. Returns true if not and it should be resorted. This is called
//    when nodes are updated and we need to see whether we need to move it.

PRBool
nsNavHistoryContainerResultNode::DoesChildNeedResorting(PRUint32 aIndex,
    SortComparator aComparator, const char* aData)
{
  NS_ASSERTION(aIndex >= 0 && aIndex < PRUint32(mChildren.Count()),
               "Input index out of range");
  if (mChildren.Count() == 1)
    return PR_FALSE;

  void* data = const_cast<void*>(static_cast<const void*>(aData));

  if (aIndex > 0) {
    // compare to previous item
    if (aComparator(mChildren[aIndex - 1], mChildren[aIndex], data) > 0)
      return PR_TRUE;
  }
  if (aIndex < PRUint32(mChildren.Count()) - 1) {
    // compare to next item
    if (aComparator(mChildren[aIndex], mChildren[aIndex + 1], data) > 0)
      return PR_TRUE;
  }
  return PR_FALSE;
}


/* static */
PRInt32 nsNavHistoryContainerResultNode::SortComparison_StringLess(
    const nsAString& a, const nsAString& b) {

  nsNavHistory* history = nsNavHistory::GetHistoryService();
  NS_ENSURE_TRUE(history, 0);
  nsICollation* collation = history->GetCollation();
  NS_ENSURE_TRUE(collation, 0);

  PRInt32 res = 0;
  collation->CompareString(nsICollation::kCollationCaseInSensitive, a, b, &res);
  return res;
}

// nsNavHistoryContainerResultNode::SortComparison_Bookmark
//
//    When there are bookmark indices, we should never have ties, so we don't
//    need to worry about tiebreaking. When there are no bookmark indices,
//    everything will be -1 and we don't worry about sorting.

PRInt32 PR_CALLBACK nsNavHistoryContainerResultNode::SortComparison_Bookmark(
    nsNavHistoryResultNode* a, nsNavHistoryResultNode* b, void* closure)
{
  return a->mBookmarkIndex - b->mBookmarkIndex;
}

// nsNavHistoryContainerResultNode::SortComparison_Title*
//
//    These are a little more complicated because they do a localization
//    conversion. If this is too slow, we can compute the sort keys once in
//    advance, sort that array, and then reorder the real array accordingly.
//    This would save some key generations.
//
//    The collation object must be allocated before sorting on title!

PRInt32 PR_CALLBACK nsNavHistoryContainerResultNode::SortComparison_TitleLess(
    nsNavHistoryResultNode* a, nsNavHistoryResultNode* b, void* closure)
{
  PRUint32 aType;
  a->GetType(&aType);

  PRInt32 value = SortComparison_StringLess(NS_ConvertUTF8toUTF16(a->mTitle),
                                            NS_ConvertUTF8toUTF16(b->mTitle));
  if (value == 0) {
    // resolve by URI
    if (a->IsURI()) {
      value = a->mURI.Compare(b->mURI.get());
    }
    if (value == 0) {
      // resolve by date
      value = ComparePRTime(a->mTime, b->mTime);
      if (value == 0)
        value = nsNavHistoryContainerResultNode::SortComparison_Bookmark(a, b, closure);
    }
  }
  return value;
}
PRInt32 PR_CALLBACK nsNavHistoryContainerResultNode::SortComparison_TitleGreater(
    nsNavHistoryResultNode* a, nsNavHistoryResultNode* b, void* closure)
{
  return -SortComparison_TitleLess(a, b, closure);
}

// nsNavHistoryContainerResultNode::SortComparison_Date*
//
//    Equal times will be very unusual, but it is important that there is some
//    deterministic ordering of the results so they don't move around.

PRInt32 PR_CALLBACK nsNavHistoryContainerResultNode::SortComparison_DateLess(
    nsNavHistoryResultNode* a, nsNavHistoryResultNode* b, void* closure)
{
  PRInt32 value = ComparePRTime(a->mTime, b->mTime);
  if (value == 0) {
    value = SortComparison_StringLess(NS_ConvertUTF8toUTF16(a->mTitle),
                                      NS_ConvertUTF8toUTF16(b->mTitle));
    if (value == 0)
      value = nsNavHistoryContainerResultNode::SortComparison_Bookmark(a, b, closure);
  }
  return value;
}
PRInt32 PR_CALLBACK nsNavHistoryContainerResultNode::SortComparison_DateGreater(
    nsNavHistoryResultNode* a, nsNavHistoryResultNode* b, void* closure)
{
  return -nsNavHistoryContainerResultNode::SortComparison_DateLess(a, b, closure);
}

// nsNavHistoryContainerResultNode::SortComparison_DateAdded*
//

PRInt32 PR_CALLBACK nsNavHistoryContainerResultNode::SortComparison_DateAddedLess(
    nsNavHistoryResultNode* a, nsNavHistoryResultNode* b, void* closure)
{
  PRInt32 value = ComparePRTime(a->mDateAdded, b->mDateAdded);
  if (value == 0) {
    value = SortComparison_StringLess(NS_ConvertUTF8toUTF16(a->mTitle),
                                      NS_ConvertUTF8toUTF16(b->mTitle));
    if (value == 0)
      value = nsNavHistoryContainerResultNode::SortComparison_Bookmark(a, b, closure);
  }
  return value;
}
PRInt32 PR_CALLBACK nsNavHistoryContainerResultNode::SortComparison_DateAddedGreater(
    nsNavHistoryResultNode* a, nsNavHistoryResultNode* b, void* closure)
{
  return -nsNavHistoryContainerResultNode::SortComparison_DateAddedLess(a, b, closure);
}


// nsNavHistoryContainerResultNode::SortComparison_LastModified*
//

PRInt32 PR_CALLBACK nsNavHistoryContainerResultNode::SortComparison_LastModifiedLess(
    nsNavHistoryResultNode* a, nsNavHistoryResultNode* b, void* closure)
{
  PRInt32 value = ComparePRTime(a->mLastModified, b->mLastModified);
  if (value == 0) {
    value = SortComparison_StringLess(NS_ConvertUTF8toUTF16(a->mTitle),
                                      NS_ConvertUTF8toUTF16(b->mTitle));
    if (value == 0)
      value = nsNavHistoryContainerResultNode::SortComparison_Bookmark(a, b, closure);
  }
  return value;
}
PRInt32 PR_CALLBACK nsNavHistoryContainerResultNode::SortComparison_LastModifiedGreater(
    nsNavHistoryResultNode* a, nsNavHistoryResultNode* b, void* closure)
{
  return -nsNavHistoryContainerResultNode::SortComparison_LastModifiedLess(a, b, closure);
}

// nsNavHistoryContainerResultNode::SortComparison_URI*
//
//    Certain types of parent nodes are treated specially because URIs are not
//    valid (like days or hosts).

PRInt32 PR_CALLBACK nsNavHistoryContainerResultNode::SortComparison_URILess(
    nsNavHistoryResultNode* a, nsNavHistoryResultNode* b, void* closure)
{
  PRInt32 value;
  if (a->IsURI() && b->IsURI()) {
    // normal URI or visit
    value = a->mURI.Compare(b->mURI.get());
  } else {
    // for everything else, use title (= host name)
    value = SortComparison_StringLess(NS_ConvertUTF8toUTF16(a->mTitle),
                                      NS_ConvertUTF8toUTF16(b->mTitle));
  }

  if (value == 0) {
    value = ComparePRTime(a->mTime, b->mTime);
    if (value == 0)
      value = nsNavHistoryContainerResultNode::SortComparison_Bookmark(a, b, closure);
  }
  return value;
}
PRInt32 PR_CALLBACK nsNavHistoryContainerResultNode::SortComparison_URIGreater(
    nsNavHistoryResultNode* a, nsNavHistoryResultNode* b, void* closure)
{
  return -SortComparison_URILess(a, b, closure);
}

// nsNavHistoryContainerResultNode::SortComparison_Keyword*
PRInt32 PR_CALLBACK nsNavHistoryContainerResultNode::SortComparison_KeywordLess(
    nsNavHistoryResultNode* a, nsNavHistoryResultNode* b, void* closure)
{
  PRInt32 value = 0;
  if (a->mItemId != -1 || b->mItemId != -1) {
    // compare the keywords
    nsAutoString aKeyword, bKeyword;
    nsNavBookmarks* bookmarks = nsNavBookmarks::GetBookmarksService();
    NS_ENSURE_TRUE(bookmarks, 0);

    nsresult rv;
    if (a->mItemId != -1) {
      rv = bookmarks->GetKeywordForBookmark(a->mItemId, aKeyword);
      NS_ENSURE_SUCCESS(rv, 0);
    }
    if (b->mItemId != -1) {
      rv = bookmarks->GetKeywordForBookmark(b->mItemId, aKeyword);
      NS_ENSURE_SUCCESS(rv, 0);
    }

    value = SortComparison_StringLess(aKeyword, bKeyword);
  }

  // fall back to title sorting
  if (value == 0)
    value = SortComparison_TitleLess(a, b, closure);

  return value;
}

PRInt32 PR_CALLBACK nsNavHistoryContainerResultNode::SortComparison_KeywordGreater(
    nsNavHistoryResultNode* a, nsNavHistoryResultNode* b, void* closure)
{
  return -SortComparison_KeywordLess(a, b, closure);
}

PRInt32 PR_CALLBACK nsNavHistoryContainerResultNode::SortComparison_AnnotationLess(
    nsNavHistoryResultNode* a, nsNavHistoryResultNode* b, void* closure)
{
  nsCAutoString annoName(static_cast<char*>(closure));
  NS_ENSURE_TRUE(!annoName.IsEmpty(), 0);
  
  nsNavBookmarks* bookmarks = nsNavBookmarks::GetBookmarksService();
  NS_ENSURE_TRUE(bookmarks, 0);

  PRBool a_itemAnno = PR_FALSE;
  PRBool b_itemAnno = PR_FALSE;

  // Not used for item annos
  nsCOMPtr<nsIURI> a_uri, b_uri;
  if (a->mItemId != -1) {
    a_itemAnno = PR_TRUE;
  } else {
    nsCAutoString spec;
    if (NS_SUCCEEDED(a->GetUri(spec)))
      NS_NewURI(getter_AddRefs(a_uri), spec);
    NS_ENSURE_TRUE(a_uri, 0);
  }

  if (b->mItemId != -1) {
    b_itemAnno = PR_TRUE;
  } else {
    nsCAutoString spec;
    if (NS_SUCCEEDED(b->GetUri(spec)))
      NS_NewURI(getter_AddRefs(b_uri), spec);
    NS_ENSURE_TRUE(b_uri, 0);
  }

  nsAnnotationService* annosvc = nsAnnotationService::GetAnnotationService();
  NS_ENSURE_TRUE(annosvc, 0);

  PRBool a_hasAnno, b_hasAnno;
  if (a_itemAnno) {
    NS_ENSURE_SUCCESS(annosvc->ItemHasAnnotation(a->mItemId, annoName,
                                                 &a_hasAnno), 0);
  } else {
    NS_ENSURE_SUCCESS(annosvc->PageHasAnnotation(a_uri, annoName,
                                                 &a_hasAnno), 0);
  }
  if (b_itemAnno) {
    NS_ENSURE_SUCCESS(annosvc->ItemHasAnnotation(b->mItemId, annoName,
                                                 &b_hasAnno), 0);
  } else {
    NS_ENSURE_SUCCESS(annosvc->PageHasAnnotation(b_uri, annoName,
                                                 &b_hasAnno), 0);    
  }

  PRInt32 value = 0;
  if (a_hasAnno || b_hasAnno) {
    PRUint16 annoType;
    if (a_hasAnno) {
      if (a_itemAnno) {
        NS_ENSURE_SUCCESS(annosvc->GetItemAnnotationType(a->mItemId,
                                                         annoName,
                                                         &annoType), 0);
      } else {
        NS_ENSURE_SUCCESS(annosvc->GetPageAnnotationType(a_uri, annoName,
                                                         &annoType), 0);
      }
    }
    if (b_hasAnno) {
      PRUint16 b_type;
      if (b_itemAnno) {
        NS_ENSURE_SUCCESS(annosvc->GetItemAnnotationType(b->mItemId,
                                                         annoName,
                                                         &b_type), 0);
      } else {
        NS_ENSURE_SUCCESS(annosvc->GetPageAnnotationType(b_uri, annoName,
                                                         &b_type), 0);
      }
      // We better make the API not support this state, really
      // XXXmano: this is actually wrong for double<->int and int64<->int32
      if (a_hasAnno && b_type != annoType)
        return 0;
      annoType = b_type;
    }

#define GET_ANNOTATIONS_VALUES(METHOD_ITEM, METHOD_PAGE, A_VAL, B_VAL)        \
        if (a_hasAnno) {                                                      \
          if (a_itemAnno) {                                                   \
            NS_ENSURE_SUCCESS(annosvc->METHOD_ITEM(a->mItemId, annoName,      \
                                                   A_VAL), 0);                \
          } else {                                                            \
            NS_ENSURE_SUCCESS(annosvc->METHOD_PAGE(a_uri, annoName,           \
                                                   A_VAL), 0);                \
          }                                                                   \
        }                                                                     \
        if (b_hasAnno) {                                                      \
          if (b_itemAnno) {                                                   \
            NS_ENSURE_SUCCESS(annosvc->METHOD_ITEM(b->mItemId, annoName,      \
                                                   B_VAL), 0);                \
          } else {                                                            \
            NS_ENSURE_SUCCESS(annosvc->METHOD_PAGE(b_uri, annoName,           \
                                                   B_VAL), 0);                \
          }                                                                   \
        }

    // Surprising as it is, we don't support sorting by a binary annotation
    if (annoType != nsIAnnotationService::TYPE_BINARY) {
      if (annoType == nsIAnnotationService::TYPE_STRING) {
        nsAutoString a_val, b_val;
        GET_ANNOTATIONS_VALUES(GetItemAnnotationString,
                               GetPageAnnotationString, a_val, b_val);
        value = SortComparison_StringLess(a_val, b_val);
      }
      else if (annoType == nsIAnnotationService::TYPE_INT32) {
        PRInt32 a_val = 0, b_val = 0;
        GET_ANNOTATIONS_VALUES(GetItemAnnotationInt32,
                               GetPageAnnotationInt32, &a_val, &b_val);
        value = (a < b) ? -1 : (a > b) ? 1 : 0;
      }
      else if (annoType == nsIAnnotationService::TYPE_INT64) {
        PRInt64 a_val = 0, b_val = 0;
        GET_ANNOTATIONS_VALUES(GetItemAnnotationInt64,
                               GetPageAnnotationInt64, &a_val, &b_val);
        value = (a < b) ? -1 : (a > b) ? 1 : 0;
      }
      else if (annoType == nsIAnnotationService::TYPE_DOUBLE) {
        double a_val = 0, b_val = 0;
        GET_ANNOTATIONS_VALUES(GetItemAnnotationDouble,
                               GetPageAnnotationDouble, &a_val, &b_val);
        value = (a < b) ? -1 : (a > b) ? 1 : 0;
      }
    }
  }

  // Note we also fall back to the title-sorting route one of the items didn't
  // have the annotation set or if both had it set but in a different storage
  // type
  if (value == 0)
    return SortComparison_TitleLess(a, b, nsnull);

  return value;
}
PRInt32 PR_CALLBACK nsNavHistoryContainerResultNode::SortComparison_AnnotationGreater(
    nsNavHistoryResultNode* a, nsNavHistoryResultNode* b, void* closure)
{
  return -SortComparison_AnnotationLess(a, b, closure);
}

// nsNavHistoryContainerResultNode::SortComparison_VisitCount*
//
//    Fall back on dates for conflict resolution

PRInt32 PR_CALLBACK nsNavHistoryContainerResultNode::SortComparison_VisitCountLess(
    nsNavHistoryResultNode* a, nsNavHistoryResultNode* b, void* closure)
{
  PRInt32 value = CompareIntegers(a->mAccessCount, b->mAccessCount);
  if (value == 0) {
    value = ComparePRTime(a->mTime, b->mTime);
    if (value == 0)
      value = nsNavHistoryContainerResultNode::SortComparison_Bookmark(a, b, closure);
  }
  return value;
}
PRInt32 PR_CALLBACK nsNavHistoryContainerResultNode::SortComparison_VisitCountGreater(
    nsNavHistoryResultNode* a, nsNavHistoryResultNode* b, void* closure)
{
  return -nsNavHistoryContainerResultNode::SortComparison_VisitCountLess(a, b, closure);
}


// nsNavHistoryContainerResultNode::SortComparison_Tags*
PRInt32 PR_CALLBACK nsNavHistoryContainerResultNode::SortComparison_TagsLess(
    nsNavHistoryResultNode* a, nsNavHistoryResultNode* b, void* closure)
{
  PRInt32 value = 0;
  nsAutoString aTags, bTags;

  nsresult rv = a->GetTags(aTags);
  NS_ENSURE_SUCCESS(rv, 0);

  rv = b->GetTags(bTags);
  NS_ENSURE_SUCCESS(rv, 0);

  value = SortComparison_StringLess(aTags, bTags);

  // fall back to title sorting
  if (value == 0)
    value = SortComparison_TitleLess(a, b, closure);

  return value;
}

PRInt32 PR_CALLBACK nsNavHistoryContainerResultNode::SortComparison_TagsGreater(
    nsNavHistoryResultNode* a, nsNavHistoryResultNode* b, void* closure)
{
  return -SortComparison_TagsLess(a, b, closure);
}

// nsNavHistoryContainerResultNode::FindChildURI
//
//    Searches this folder for a node with the given URI. Returns null if not
//    found. Does not addref the node!

nsNavHistoryResultNode*
nsNavHistoryContainerResultNode::FindChildURI(const nsACString& aSpec,
    PRUint32* aNodeIndex)
{
  for (PRInt32 i = 0; i < mChildren.Count(); i ++) {
    if (mChildren[i]->IsURI()) {
      if (aSpec.Equals(mChildren[i]->mURI)) {
        *aNodeIndex = i;
        return mChildren[i];
      }
    }
  }
  return nsnull;
}


// nsNavHistoryContainerResultNode::FindChildFolder
//
//    Searches this folder for the given subfolder. Returns null if not found.
//    DOES NOT ADDREF.

nsNavHistoryFolderResultNode*
nsNavHistoryContainerResultNode::FindChildFolder(PRInt64 aFolderId,
    PRUint32* aNodeIndex)
{
  for (PRInt32 i = 0; i < mChildren.Count(); i ++) {
    if (mChildren[i]->IsFolder()) {
      nsNavHistoryFolderResultNode* folder = mChildren[i]->GetAsFolder();
      if (folder->mItemId == aFolderId) {
        *aNodeIndex = i;
        return folder;
      }
    }
  }
  return nsnull;
}


//  nsNavHistoryContainerResultNode::FindChildContainerByName
//
//    Searches this container for a subfolder with the given name. This is used
//    to find host and "day" nodes. Returns null if not found. DOES NOT ADDREF.

nsNavHistoryContainerResultNode*
nsNavHistoryContainerResultNode::FindChildContainerByName(
    const nsACString& aTitle, PRUint32* aNodeIndex)
{
  for (PRInt32 i = 0; i < mChildren.Count(); i ++) {
    if (mChildren[i]->IsContainer()) {
      nsNavHistoryContainerResultNode* container =
          mChildren[i]->GetAsContainer();
      if (container->mTitle.Equals(aTitle)) {
        *aNodeIndex = i;
        return container;
      }
    }
  }
  return nsnull;
}


// nsNavHistoryContainerResultNode::InsertChildAt
//
//    This does the work of adding a child to the container. This child can be
//    a container, or a single item that may even be collapsed with the
//    adjacent ones.
//
//    Some inserts are "temporary" meaning that they are happening immediately
//    after a temporary remove. We do this when movings elements when they
//    change to keep them in the proper sorting position. In these cases, we
//    don't need to recompute any statistics.

nsresult
nsNavHistoryContainerResultNode::InsertChildAt(nsNavHistoryResultNode* aNode,
                                               PRInt32 aIndex,
                                               PRBool aIsTemporary)
{
  nsNavHistoryResult* result = GetResult();
  NS_ENSURE_TRUE(result, NS_ERROR_FAILURE);

  aNode->mViewIndex = -1;
  aNode->mParent = this;
  aNode->mIndentLevel = mIndentLevel + 1;
  if (! aIsTemporary && aNode->IsContainer()) {
    // need to update all the new item's children
    nsNavHistoryContainerResultNode* container = aNode->GetAsContainer();
    container->mResult = mResult;
    container->FillStats();
  }

  if (! mChildren.InsertObjectAt(aNode, aIndex))
    return NS_ERROR_OUT_OF_MEMORY;

  // update our (the container) stats and refresh our row on the screen
  if (! aIsTemporary) {
    mAccessCount += aNode->mAccessCount;
    if (mTime < aNode->mTime)
      mTime = aNode->mTime;
    if (result->GetView() && (!mParent || mParent->AreChildrenVisible()))
      result->GetView()->ItemChanged(
          static_cast<nsINavHistoryContainerResultNode*>(this));
    ReverseUpdateStats(aNode->mAccessCount);
  }

  // Update tree if we are visible. Note that we could be here and not expanded,
  // like when there is a bookmark folder being updated because its parent is
  // visible.
  if (result->GetView() && AreChildrenVisible())
    result->GetView()->ItemInserted(this, aNode, aIndex);
  return NS_OK;
}


// nsNavHistoryContainerResultNode::InsertSortedChild
//
//    This locates the proper place for insertion according to the current sort
//    and calls InsertChildAt

nsresult
nsNavHistoryContainerResultNode::InsertSortedChild(
    nsNavHistoryResultNode* aNode, 
    PRBool aIsTemporary, PRBool aIgnoreDuplicates)
{

  if (mChildren.Count() == 0)
    return InsertChildAt(aNode, 0, aIsTemporary);

  SortComparator comparator = GetSortingComparator(GetSortType());
  if (comparator) {
    // When inserting a new node, it must have proper statistics because we use
    // them to find the correct insertion point. The insert function will then
    // recompute these statistics and fill in the proper parents and hierarchy
    // level. Doing this twice shouldn't be a large performance penalty because
    // when we are inserting new containers, they typically contain only one
    // item (because we've browsed a new page).
    if (! aIsTemporary && aNode->IsContainer()) {
      // need to update all the new item's children
      nsNavHistoryContainerResultNode* container = aNode->GetAsContainer();
      container->mResult = mResult;
      container->FillStats();
    }

    nsCAutoString sortingAnnotation;
    GetSortingAnnotation(sortingAnnotation);
    PRBool itemExists;
    PRUint32 position = FindInsertionPoint(aNode, comparator, 
                                           sortingAnnotation.get(), 
                                           &itemExists);
    if (aIgnoreDuplicates && itemExists)
      return NS_OK;

    return InsertChildAt(aNode, position, aIsTemporary);
  }
  return InsertChildAt(aNode, mChildren.Count(), aIsTemporary);
}

// nsNavHistoryContainerResultNode::EnsureItemPosition
//
//  This checks if the item at aIndex is located correctly given the sorting
//  move. If it's not, the item is moved, and the result view are notified.
//
//  Returns true if the item position has been changed, false otherwise.

PRBool
nsNavHistoryContainerResultNode::EnsureItemPosition(PRUint32 aIndex) {
  NS_ASSERTION(aIndex >= 0 && aIndex < mChildren.Count(), "Invalid index");
  if (aIndex < 0 || aIndex >= mChildren.Count())
    return PR_FALSE;

  SortComparator comparator = GetSortingComparator(GetSortType());
  if (!comparator)
    return PR_FALSE;

  nsCAutoString sortAnno;
  GetSortingAnnotation(sortAnno);
  if (!DoesChildNeedResorting(aIndex, comparator, sortAnno.get()))
    return PR_FALSE;

  nsRefPtr<nsNavHistoryResultNode> node(mChildren[aIndex]);
  mChildren.RemoveObjectAt(aIndex);

  PRUint32 newIndex = FindInsertionPoint(
                          node, comparator,sortAnno.get(), nsnull);
  mChildren.InsertObjectAt(node.get(), newIndex);

  nsNavHistoryResult* result = GetResult();
  NS_ENSURE_TRUE(result, PR_TRUE);

  if (result->GetView() && AreChildrenVisible())
    result->GetView()->ItemMoved(node, this, aIndex, this, newIndex);

  return PR_TRUE;
}

// nsNavHistoryContainerResultNode::MergeResults
//
//    This takes a list of nodes and merges them into the current result set.
//    Any containers that are added must already be sorted.
//
//    This assumes that the items in 'aAddition' are new visits or
//    replacement URIs. We do not update visits.
//
//    PERFORMANCE: In the future, we can do more updates incrementally using
//    this function. When a URI changes in a way we can't easily handle,
//    construct a query with each query object specifying an exact match for
//    the URI in question. Then remove all instances of that URI in the result
//    and call this function.

void
nsNavHistoryContainerResultNode::MergeResults(
    nsCOMArray<nsNavHistoryResultNode>* aAddition)
{
  // Generally we will have very few (one) entries in the addition list, so
  // just iterate through it. If we find we may have a lot, we may want to do
  // some hashing to help with the merge.
  for (PRUint32 i = 0; i < PRUint32(aAddition->Count()); i ++) {
    nsNavHistoryResultNode* curAddition = (*aAddition)[i];
    if (curAddition->IsContainer()) {
      PRUint32 containerIndex;
      nsNavHistoryContainerResultNode* container =
        FindChildContainerByName(curAddition->mTitle, &containerIndex);
      if (container) {
        // recursively merge with the existing container
        container->MergeResults(&curAddition->GetAsContainer()->mChildren);
      } else {
        // need to add the new container to our result.
        InsertSortedChild(curAddition);
      }
    } else {
      if (curAddition->IsVisit()) {
        // insert the new visit
        InsertSortedChild(curAddition);
      } else {
        // add the URI, replacing a current one if any
        PRUint32 oldIndex;
        nsNavHistoryResultNode* oldNode =
          FindChildURI(curAddition->mURI, &oldIndex);
        if (oldNode) {
          // if we don't have a parent (for example, the history
          // sidebar, when sorted by last visited or most visited)
          // we have to manually Remove/Insert instead of Replace
          // see bug #389782 for details
          if (mParent)
            ReplaceChildURIAt(oldIndex, curAddition);
          else {
            RemoveChildAt(oldIndex, PR_TRUE);
            InsertSortedChild(curAddition, PR_TRUE);
          }
        }
        else
          InsertSortedChild(curAddition);
      }
    }
  }
}


// nsNavHistoryContainerResultNode::ReplaceChildURIAt
//
//    This is called to replace a leaf node. It will update tree stats
//    and redraw the view if any. You can not use this to replace a container.
//
//    This assumes that the node is being replaced with a newer version of
//    itself and so its visit count will not go down. Also, this means that the
//    collapsing of duplicates will not change.

nsresult
nsNavHistoryContainerResultNode::ReplaceChildURIAt(PRUint32 aIndex,
    nsNavHistoryResultNode* aNode)
{
  NS_ASSERTION(aIndex < PRUint32(mChildren.Count()),
               "Invalid index for replacement");
  NS_ASSERTION(mChildren[aIndex]->IsURI(),
               "Can not use ReplaceChildAt for a node of another type");
  NS_ASSERTION(mChildren[aIndex]->mURI.Equals(aNode->mURI),
               "We must replace a URI with an updated one of the same");

  aNode->mParent = this;
  aNode->mIndentLevel = mIndentLevel + 1;

  // update tree stats if needed
  PRUint32 accessCountChange = aNode->mAccessCount - mChildren[aIndex]->mAccessCount;
  if (accessCountChange != 0 || mChildren[aIndex]->mTime != aNode->mTime) {
    NS_ASSERTION(aNode->mAccessCount >= mChildren[aIndex]->mAccessCount,
                 "Replacing a node with one back in time or some nonmatching node");

    mAccessCount += accessCountChange;
    if (mTime < aNode->mTime)
      mTime = aNode->mTime;
    ReverseUpdateStats(accessCountChange);
  }

  // Hold a reference so it doesn't go away as soon as we remove it from the
  // array. This needs to be passed to the view.
  nsRefPtr<nsNavHistoryResultNode> oldItem = mChildren[aIndex];

  // actually replace
  if (! mChildren.ReplaceObjectAt(aNode, aIndex))
    return NS_ERROR_FAILURE;

  // update view
  nsNavHistoryResult* result = GetResult();
  NS_ENSURE_TRUE(result, NS_ERROR_FAILURE);
  if (result->GetView() && AreChildrenVisible())
    result->GetView()->ItemReplaced(this, oldItem, aNode, aIndex);

  mChildren[aIndex]->OnRemoving();
  return NS_OK;
}


// nsNavHistoryContainerResultNode::RemoveChildAt
//
//    This does all the work of removing a child from this container, including
//    updating the tree if necessary. Note that we do not need to be open for
//    this to work.
//
//    Some removes are "temporary" meaning that they'll just get inserted
//    again. We do this for resorting. In these cases, we don't need to
//    recompute any statistics, and we shouldn't notify those container that
//    they are being removed.

nsresult
nsNavHistoryContainerResultNode::RemoveChildAt(PRInt32 aIndex,
                                               PRBool aIsTemporary)
{
  NS_ASSERTION(aIndex >= 0 && aIndex < mChildren.Count(), "Invalid index");

  nsNavHistoryResult* result = GetResult();
  NS_ENSURE_TRUE(result, NS_ERROR_FAILURE);

  // hold an owning reference to keep from expiring while we work with it
  nsRefPtr<nsNavHistoryResultNode> oldNode = mChildren[aIndex];

  // stats
  PRUint32 oldAccessCount = 0;
  if (! aIsTemporary) {
    oldAccessCount = mAccessCount;
    mAccessCount -= mChildren[aIndex]->mAccessCount;
    NS_ASSERTION(mAccessCount >= 0, "Invalid access count while updating!");
  }

  // remove from our list and notify the tree
  mChildren.RemoveObjectAt(aIndex);
  if (result->GetView() && AreChildrenVisible())
    result->GetView()->ItemRemoved(this, oldNode, aIndex);

  if (! aIsTemporary) {
    ReverseUpdateStats(mAccessCount - oldAccessCount);
    oldNode->OnRemoving();
  }
  return NS_OK;
}


// nsNavHistoryContainerResultNode::RecursiveFindURIs
//
//    This function searches for matches for the given URI.
//
//    If aOnlyOne is set, it will terminate as soon as it finds a single match.
//    This would be used when there are URI results so there will only ever be
//    one copy of any URI.
//
//    When aOnlyOne is false, it will check all elements. This is for visit
//    style results that may have multiple copies of any given URI.

void
nsNavHistoryContainerResultNode::RecursiveFindURIs(PRBool aOnlyOne,
    nsNavHistoryContainerResultNode* aContainer, const nsCString& aSpec,
    nsCOMArray<nsNavHistoryResultNode>* aMatches)
{
  for (PRInt32 child = 0; child < aContainer->mChildren.Count(); child ++) {
    PRUint32 type;
    aContainer->mChildren[child]->GetType(&type);
    if (nsNavHistoryResultNode::IsTypeURI(type)) {
      // compare URIs
      nsNavHistoryResultNode* uriNode = aContainer->mChildren[child];
      if (uriNode->mURI.Equals(aSpec)) {
        // found
        aMatches->AppendObject(uriNode);
        if (aOnlyOne)
          return;
      }
    }
  }
}


// nsNavHistoryContainerResultNode::UpdateURIs
//
//    If aUpdateSort is true, we will also update the sorting of this item.
//    Normally you want this to be true, but it can be false if the thing you
//    are changing can not affect sorting (like favicons).
//
//    You should NOT change any child lists as part of the callback function.

void
nsNavHistoryContainerResultNode::UpdateURIs(PRBool aRecursive, PRBool aOnlyOne,
    PRBool aUpdateSort, const nsCString& aSpec,
    void (*aCallback)(nsNavHistoryResultNode*,void*), void* aClosure)
{
  nsNavHistoryResult* result = GetResult();
  if (! result) {
    NS_NOTREACHED("Must have a result for this query");
    return;
  }

  // this needs to be owning since sometimes we remove and re-insert nodes
  // in their parents and we don't want them to go away.
  nsCOMArray<nsNavHistoryResultNode> matches;

  if (aRecursive) {
    RecursiveFindURIs(aOnlyOne, this, aSpec, &matches);
  } else if (aOnlyOne) {
    PRUint32 nodeIndex;
    nsNavHistoryResultNode* node = FindChildURI(aSpec, &nodeIndex);
    if (node)
      matches.AppendObject(node);
  } else {
    NS_NOTREACHED("UpdateURIs does not handle nonrecursive updates of multiple items.");
    // this case easy to add if you need it, just find all the matching URIs
    // at this level. However, this isn't currently used. History uses recursive,
    // Bookmarks uses one level and knows that the match is unique.
    return;
  }
  if (matches.Count() == 0)
    return;

  SortComparator comparator = nsnull;
  nsCAutoString sortingAnnotation;
  if (aUpdateSort) {
    comparator = GetSortingComparator(GetSortType());
    GetSortingAnnotation(sortingAnnotation);
  }

  // PERFORMANCE: This updates each container for each child in it that
  // changes. In some cases, many elements have changed inside the same
  // container. It would be better to compose a list of containers, and
  // update each one only once for all the items that have changed in it.
  for (PRInt32 i = 0; i < matches.Count(); i ++)
  {
    nsNavHistoryResultNode* node = matches[i];
    nsNavHistoryContainerResultNode* parent = node->mParent;
    if (! parent) {
      NS_NOTREACHED("All URI nodes being updated must have parents");
      continue;
    }
    PRBool childrenVisible = result->GetView() != nsnull && parent->AreChildrenVisible();

    PRUint32 oldAccessCount = node->mAccessCount;
    PRTime oldTime = node->mTime;
    aCallback(node, aClosure);

    if (oldAccessCount != node->mAccessCount || oldTime != node->mTime) {
      // need to update/redraw the parent
      parent->mAccessCount += node->mAccessCount - oldAccessCount;
      if (node->mTime > parent->mTime)
        parent->mTime = node->mTime;
      if (childrenVisible)
        result->GetView()->ItemChanged(
            static_cast<nsINavHistoryContainerResultNode*>(parent));
      parent->ReverseUpdateStats(node->mAccessCount - oldAccessCount);
    }

    if (aUpdateSort) {
      PRInt32 childIndex = parent->FindChild(node);
      if ((childIndex < 0 || !parent->EnsureItemPosition(childIndex) && childrenVisible)) {
        result->GetView()->ItemChanged(node);
      }
    } else if (childrenVisible) {
      result->GetView()->ItemChanged(node);
    }
  }
}


// nsNavHistoryContainerResultNode::ChangeTitles
//
//    This is used to update the titles in the tree. This is called from both
//    query and bookmark folder containers to update the tree. Bookmark folders
//    should be sure to set recursive to false, since child folders will have
//    their own callbacks registered.

static void PR_CALLBACK setTitleCallback(
    nsNavHistoryResultNode* aNode, void* aClosure)
{
  const nsACString* newTitle = reinterpret_cast<nsACString*>(aClosure);
  aNode->mTitle = *newTitle;
}
nsresult
nsNavHistoryContainerResultNode::ChangeTitles(nsIURI* aURI,
                                              const nsACString& aNewTitle,
                                              PRBool aRecursive,
                                              PRBool aOnlyOne)
{
  // uri string
  nsCAutoString uriString;
  nsresult rv = aURI->GetSpec(uriString);
  NS_ENSURE_SUCCESS(rv, rv);

  // The recursive function will update the result's tree nodes, but only if we
  // give it a non-null pointer. So if there isn't a tree, just pass NULL so
  // it doesn't bother trying to call the result.
  nsNavHistoryResult* result = GetResult();
  NS_ENSURE_TRUE(result, NS_ERROR_FAILURE);

  PRUint16 sortType = GetSortType();
  PRBool updateSorting =
    (sortType == nsINavHistoryQueryOptions::SORT_BY_TITLE_ASCENDING ||
     sortType == nsINavHistoryQueryOptions::SORT_BY_TITLE_DESCENDING);

  UpdateURIs(aRecursive, aOnlyOne, updateSorting, uriString,
             setTitleCallback,
             const_cast<void*>(reinterpret_cast<const void*>(&aNewTitle)));
  return NS_OK;
}


// nsNavHistoryContainerResultNode::GetHasChildren
//
//    Complex containers (folders and queries) will override this. Here, we
//    handle the case of simple containers (like host groups) where the children
//    are always stored.

NS_IMETHODIMP
nsNavHistoryContainerResultNode::GetHasChildren(PRBool *aHasChildren)
{
  *aHasChildren = (mChildren.Count() > 0);
  return NS_OK;
}


// nsNavHistoryContainerResultNode::GetChildCount
//
//    This function is only valid when the node is opened.

NS_IMETHODIMP
nsNavHistoryContainerResultNode::GetChildCount(PRUint32* aChildCount)
{
  if (! mExpanded)
    return NS_ERROR_NOT_AVAILABLE;
  *aChildCount = mChildren.Count();
  return NS_OK;
}


// nsNavHistoryContainerResultNode::GetChild

NS_IMETHODIMP
nsNavHistoryContainerResultNode::GetChild(PRUint32 aIndex,
                                          nsINavHistoryResultNode** _retval)
{
  if (! mExpanded)
    return NS_ERROR_NOT_AVAILABLE;
  if (aIndex >= PRUint32(mChildren.Count()))
    return NS_ERROR_INVALID_ARG;
  NS_ADDREF(*_retval = mChildren[aIndex]);
  return NS_OK;
}


// nsNavHistoryContainerResultNode::GetChildrenReadOnly
//
//    Overridden for folders to query the bookmarks service directly.

NS_IMETHODIMP
nsNavHistoryContainerResultNode::GetChildrenReadOnly(PRBool *aChildrenReadOnly)
{
  *aChildrenReadOnly = mChildrenReadOnly;
  return NS_OK;
}


// nsNavHistoryContainerResultNode::GetDynamicContainerType

NS_IMETHODIMP
nsNavHistoryContainerResultNode::GetDynamicContainerType(
    nsACString& aDynamicContainerType)
{
  aDynamicContainerType = mDynamicContainerType;
  return NS_OK;
}


// nsNavHistoryContainerResultNode::AppendURINode

NS_IMETHODIMP
nsNavHistoryContainerResultNode::AppendURINode(
    const nsACString& aURI, const nsACString& aTitle, PRUint32 aAccessCount,
    PRTime aTime, const nsACString& aIconURI, nsINavHistoryResultNode** _retval)
{
  *_retval = nsnull;
  if (!IsDynamicContainer())
    return NS_ERROR_INVALID_ARG; // we must be a dynamic container

  nsRefPtr<nsNavHistoryResultNode> result =
      new nsNavHistoryResultNode(aURI, aTitle, aAccessCount, aTime, aIconURI);
  NS_ENSURE_TRUE(result, NS_ERROR_OUT_OF_MEMORY);

  // append to our list
  nsresult rv = InsertChildAt(result, mChildren.Count());
  NS_ENSURE_SUCCESS(rv, rv);

  NS_ADDREF(*_retval = result);
  return NS_OK;
}


// nsNavHistoryContainerResultNode::AppendVisitNode

#if 0 // UNTESTED, commented out until it can be tested
NS_IMETHODIMP
nsNavHistoryContainerResultNode::AppendVisitNode(
    const nsACString& aURI, const nsACString& aTitle, PRUint32 aAccessCount,
    PRTime aTime, const nsACString& aIconURI, PRInt64 aSession,
    nsINavHistoryVisitResultNode** _retval)
{
  *_retval = nsnull;
  if (!IsDynamicContainer())
    return NS_ERROR_INVALID_ARG; // we must be a dynamic container

  nsRefPtr<nsNavHistoryVisitResultNode> result =
      new nsNavHistoryVisitResultNode(aURI, aTitle, aAccessCount, aTime,
                                      aIconURI, aSession);
  NS_ENSURE_TRUE(result, NS_ERROR_OUT_OF_MEMORY);

  // append to our list
  if (! mChildren.AppendObject(result))
    return NS_ERROR_OUT_OF_MEMORY;
  NS_ADDREF(*_retval = result);
  return NS_OK;
}


// nsNavHistoryContainerResultNode::AppendFullVisitNode

NS_IMETHODIMP
nsNavHistoryContainerResultNode::AppendFullVisitNode(
    const nsACString& aURI, const nsACString& aTitle, PRUint32 aAccessCount,
    PRTime aTime, const nsACString& aIconURI, PRInt64 aSession,
    PRInt64 aVisitId, PRInt64 aReferringVisitId, PRInt32 aTransitionType,
    nsINavHistoryFullVisitResultNode** _retval)
{
  *_retval = nsnull;
  if (!IsDynamicContainer())
    return NS_ERROR_INVALID_ARG; // we must be a dynamic container

  nsRefPtr<nsNavHistoryFullVisitResultNode> result =
      new nsNavHistoryFullVisitResultNode(aURI, aTitle, aAccessCount, aTime,
                                          aIconURI, aSession, aVisitId,
                                          aReferringVisitId, aTransitionType);
  NS_ENSURE_TRUE(result, NS_ERROR_OUT_OF_MEMORY);

  // append to our list
  if (! mChildren.AppendObject(result))
    return NS_ERROR_OUT_OF_MEMORY;
  NS_ADDREF(*_retval = result);
  return NS_OK;
}


// nsNavHistoryContainerResultNode::AppendContainerNode

NS_IMETHODIMP
nsNavHistoryContainerResultNode::AppendContainerNode(
    const nsACString& aTitle, const nsACString& aIconURI,
    PRUint32 aContainerType, const nsACString& aDynamicContainerType,
    nsINavHistoryContainerResultNode** _retval)
{
  *_retval = nsnull;
  if (!IsDynamicContainer())
    return NS_ERROR_INVALID_ARG; // we must be a dynamic container
  if (! IsTypeContainer(aContainerType) || IsTypeFolder(aContainerType) ||
      IsTypeQuery(aContainerType))
    return NS_ERROR_INVALID_ARG; // not proper container type
  if (aContainerType == nsNavHistoryResultNode::RESULT_TYPE_DYNAMIC_CONTAINER &&
      aRemoteContainerType.IsEmpty())
    return NS_ERROR_INVALID_ARG; // dynamic containers must have d.c. type
  if (aContainerType != nsNavHistoryResultNode::RESULT_TYPE_DYNAMIC_CONTAINER &&
      ! aDynamicContainerType.IsEmpty())
    return NS_ERROR_INVALID_ARG; // non-dynamic containers must NOT have d.c. type

  nsRefPtr<nsNavHistoryContainerResultNode> result =
      new nsNavHistoryContainerResultNode(EmptyCString(), aTitle, aIconURI,
                                          aContainerType, PR_TRUE,
                                          aDynamicContainerType);
  NS_ENSURE_TRUE(result, NS_ERROR_OUT_OF_MEMORY);

  // append to our list
  if (! mChildren.AppendObject(result))
    return NS_ERROR_OUT_OF_MEMORY;
  NS_ADDREF(*_retval = result);
  return NS_OK;
}


// nsNavHistoryContainerResultNode::AppendQueryNode

NS_IMETHODIMP
nsNavHistoryContainerResultNode::AppendQueryNode(
    const nsACString& aQueryURI, const nsACString& aTitle,
    const nsACString& aIconURI, nsINavHistoryQueryResultNode** _retval)
{
  *_retval = nsnull;
  if (!IsDynamicContainer())
    return NS_ERROR_INVALID_ARG; // we must be a dynamic container

  nsRefPtr<nsNavHistoryQueryResultNode> result =
      new nsNavHistoryQueryResultNode(aQueryURI, aTitle, aIconURI);
  NS_ENSURE_TRUE(result, NS_ERROR_OUT_OF_MEMORY);

  // append to our list
  if (! mChildren.AppendObject(result))
    return NS_ERROR_OUT_OF_MEMORY;
  NS_ADDREF(*_retval = result);
  return NS_OK;
}
#endif

// nsNavHistoryContainerResultNode::AppendFolderNode

NS_IMETHODIMP
nsNavHistoryContainerResultNode::AppendFolderNode(
    PRInt64 aFolderId, nsINavHistoryContainerResultNode** _retval)
{
  *_retval = nsnull;
  if (!IsDynamicContainer())
    return NS_ERROR_INVALID_ARG; // we must be a dynamic container

  nsNavBookmarks* bookmarks = nsNavBookmarks::GetBookmarksService();
  NS_ENSURE_TRUE(bookmarks, NS_ERROR_OUT_OF_MEMORY);

  // create the node, it will be addrefed for us
  nsRefPtr<nsNavHistoryResultNode> result;
  nsresult rv = bookmarks->ResultNodeForContainer(aFolderId,
                                                  GetGeneratingOptions(),
                                                  getter_AddRefs(result));
  NS_ENSURE_SUCCESS(rv, rv);

  // append to our list
  rv = InsertChildAt(result, mChildren.Count());
  NS_ENSURE_SUCCESS(rv, rv);

  NS_ADDREF(*_retval = result->GetAsContainer());
  return NS_OK;
}


// nsNavHistoryContainerResultNode::ClearContents
//
//    Used by the dynamic container API to clear this container

#if 0 // UNTESTED, commented out until it can be tested
NS_IMETHODIMP
nsNavHistoryContainerResultNode::ClearContents()
{
  if (!IsDynamicContainer())
    return NS_ERROR_INVALID_ARG; // we must be a dynamic container

  // we know if CanRemoteContainersChange() then we are a regular container
  // and not a query or folder, so clearing doesn't need anything else to
  // happen (like unregistering observers). Also, since this should only
  // happen when the container is closed, we don't need to redraw the screen.
  mChildren.Clear();

  PRUint32 oldAccessCount = mAccessCount;
  mAccessCount = 0;
  mTime = 0;
  ReverseUpdateStats(-PRInt32(oldAccessCount));
  return NS_OK;
}
#endif

// nsNavHistoryQueryResultNode *************************************************
//
//    HOW QUERY UPDATING WORKS
//
//    Queries are different than bookmark folders in that we can not always
//    do dynamic updates (easily) and updates are more expensive. Therefore,
//    we do NOT query if we are not open and want to see if we have any children
//    (for drawing a twisty) and always assume we will.
//
//    When the container is opened, we execute the query and register the
//    listeners. Like bookmark folders, we stay registered even when closed,
//    and clear ourselves as soon as a message comes in. This lets us respond
//    quickly if the user closes and reopens the container.
//
//    We try to handle the most common notifications for the most common query
//    types dynamically, that is, figuring out what should happen in response
//    to a message without doing a requery. For complex changes or complex
//    queries, we give up and requery.

NS_IMPL_ISUPPORTS_INHERITED1(nsNavHistoryQueryResultNode,
                             nsNavHistoryContainerResultNode,
                             nsINavHistoryQueryResultNode)

nsNavHistoryQueryResultNode::nsNavHistoryQueryResultNode(
    const nsACString& aTitle, const nsACString& aIconURI,
    const nsACString& aQueryURI) :
  nsNavHistoryContainerResultNode(aQueryURI, aTitle, aIconURI,
                                  nsNavHistoryResultNode::RESULT_TYPE_QUERY,
                                  PR_TRUE, EmptyCString(), nsnull),
  mHasSearchTerms(PR_FALSE),
  mContentsValid(PR_FALSE),
  mBatchInProgress(PR_FALSE)
{
}

nsNavHistoryQueryResultNode::nsNavHistoryQueryResultNode(
    const nsACString& aTitle, const nsACString& aIconURI,
    const nsCOMArray<nsNavHistoryQuery>& aQueries,
    nsNavHistoryQueryOptions* aOptions) :
  nsNavHistoryContainerResultNode(EmptyCString(), aTitle, aIconURI,
                                  nsNavHistoryResultNode::RESULT_TYPE_QUERY,
                                  PR_TRUE, EmptyCString(), aOptions),
  mQueries(aQueries),
  mContentsValid(PR_FALSE),
  mBatchInProgress(PR_FALSE)
{
  NS_ASSERTION(aQueries.Count() > 0, "Must have at least one query");

  nsNavHistory* history = nsNavHistory::GetHistoryService();
  NS_ASSERTION(history, "History service missing");
  mLiveUpdate = history->GetUpdateRequirements(mQueries, mOptions,
                                               &mHasSearchTerms);
}

nsNavHistoryQueryResultNode::nsNavHistoryQueryResultNode(
    const nsACString& aTitle, const nsACString& aIconURI,
    PRTime aTime,
    const nsCOMArray<nsNavHistoryQuery>& aQueries,
    nsNavHistoryQueryOptions* aOptions) :
  nsNavHistoryContainerResultNode(EmptyCString(), aTitle, aTime, aIconURI,
                                  nsNavHistoryResultNode::RESULT_TYPE_QUERY,
                                  PR_TRUE, EmptyCString(), aOptions),
  mQueries(aQueries),
  mContentsValid(PR_FALSE),
  mBatchInProgress(PR_FALSE)
{
  NS_ASSERTION(aQueries.Count() > 0, "Must have at least one query");

  nsNavHistory* history = nsNavHistory::GetHistoryService();
  NS_ASSERTION(history, "History service missing");
  mLiveUpdate = history->GetUpdateRequirements(mQueries, mOptions,
                                               &mHasSearchTerms);
}


// nsNavHistoryQueryResultNode::CanExpand
//
//    Whoever made us may want non-expanding queries. However, we always
//    expand when we are the root node, or else asking for non-expanding
//    queries would be useless. A query node is not expandable if excludeItems=1
//    or expandQueries=0.

PRBool
nsNavHistoryQueryResultNode::CanExpand()
{
  nsNavHistoryQueryOptions* options = GetGeneratingOptions();
  if (options) {
    if (options->ExcludeItems())
      return PR_FALSE;
    if (options->ExpandQueries())
      return PR_TRUE;
  }
  if (mResult && mResult->mRootNode == this)
    return PR_TRUE;
  return PR_FALSE;
}


// nsNavHistoryQueryResultNode::OnRemoving
//
//    Here we do not want to call ContainerResultNode::OnRemoving since our own
//    ClearChildren will do the same thing and more (unregister the observers).
//    The base ResultNode::OnRemoving will clear some regular node stats, so it
//    is OK.

void
nsNavHistoryQueryResultNode::OnRemoving()
{
  nsNavHistoryResultNode::OnRemoving();
  ClearChildren(PR_TRUE);
}


// nsNavHistoryQueryResultNode::OpenContainer
//
//    Marks the container as open, rebuilding results if they are invalid. We
//    may still have valid results if the container was previously open and
//    nothing happened since closing it.
//
//    We do not handle CloseContainer specially. The default one just marks the
//    container as closed, but doesn't actually mark the results as invalid.
//    The results will be invalidated by the next history or bookmark
//    notification that comes in. This means if you open and close the item
//    without anything happening in between, it will be fast (this actually
//    happens when results are used as menus).

nsresult
nsNavHistoryQueryResultNode::OpenContainer()
{
  NS_ASSERTION(!mExpanded, "Container must be closed to open it");
  mExpanded = PR_TRUE;
  if (!CanExpand())
    return NS_OK;
  if (!mContentsValid) {
    nsresult rv = FillChildren();
    NS_ENSURE_SUCCESS(rv, rv);
  }

  nsNavHistoryResult* result = GetResult();
  NS_ENSURE_TRUE(result, NS_ERROR_FAILURE);
  if (result->GetView())
    result->GetView()->ContainerOpened(
        static_cast<nsNavHistoryContainerResultNode*>(this));
  return NS_OK;
}


// nsNavHistoryQueryResultNode::GetHasChildren
//
//    When we have valid results we can always give an exact answer. When we
//    don't we just assume we'll have results, since actually doing the query
//    might be hard. This is used to draw twisties on the tree, so precise
//    results don't matter.

NS_IMETHODIMP
nsNavHistoryQueryResultNode::GetHasChildren(PRBool* aHasChildren)
{
  if (! CanExpand()) {
    *aHasChildren = PR_FALSE;
    return NS_OK;
  }
  if (mContentsValid) {
    *aHasChildren = (mChildren.Count() > 0);
    return NS_OK;
  }
  *aHasChildren = PR_TRUE;
  return NS_OK;
}


// nsNavHistoryQueryResultNode::GetUri
//
//    This doesn't just return mURI because in the case of queries that may
//    be lazily constructed from the query objects.

NS_IMETHODIMP
nsNavHistoryQueryResultNode::GetUri(nsACString& aURI)
{
  nsresult rv = VerifyQueriesSerialized();
  NS_ENSURE_SUCCESS(rv, rv);
  aURI = mURI;
  return NS_OK;
}

// nsNavHistoryQueryResultNode::GetFolderItemId

NS_IMETHODIMP
nsNavHistoryQueryResultNode::GetFolderItemId(PRInt64* aItemId)
{
  *aItemId = mItemId;
  return NS_OK;
}

// nsNavHistoryQueryResultNode::GetQueries

NS_IMETHODIMP
nsNavHistoryQueryResultNode::GetQueries(PRUint32* queryCount,
                                        nsINavHistoryQuery*** queries)
{
  nsresult rv = VerifyQueriesParsed();
  NS_ENSURE_SUCCESS(rv, rv);
  NS_ASSERTION(mQueries.Count() > 0, "Must have >= 1 query");

  *queries = static_cast<nsINavHistoryQuery**>
                        (nsMemory::Alloc(mQueries.Count() * sizeof(nsINavHistoryQuery*)));
  NS_ENSURE_TRUE(*queries, NS_ERROR_OUT_OF_MEMORY);

  for (PRInt32 i = 0; i < mQueries.Count(); ++i)
    NS_ADDREF((*queries)[i] = mQueries[i]);
  *queryCount = mQueries.Count();
  return NS_OK;
}


// nsNavHistoryQueryResultNode::GetQueryOptions

NS_IMETHODIMP
nsNavHistoryQueryResultNode::GetQueryOptions(
                                      nsINavHistoryQueryOptions** aQueryOptions)
{
  nsresult rv = VerifyQueriesParsed();
  NS_ENSURE_SUCCESS(rv, rv);
  NS_ASSERTION(mOptions, "Options invalid");

  *aQueryOptions = mOptions;
  NS_ADDREF(*aQueryOptions);
  return NS_OK;
}


// nsNavHistoryQueryResultNode::VerifyQueriesParsed

nsresult
nsNavHistoryQueryResultNode::VerifyQueriesParsed()
{
  if (mQueries.Count() > 0) {
    NS_ASSERTION(mOptions, "If a result has queries, it also needs options");
    return NS_OK;
  }
  NS_ASSERTION(! mURI.IsEmpty(),
               "Query nodes must have either a URI or query/options");

  nsNavHistory* history = nsNavHistory::GetHistoryService();
  NS_ENSURE_TRUE(history, NS_ERROR_OUT_OF_MEMORY);

  nsresult rv = history->QueryStringToQueryArray(mURI, &mQueries,
                                                 getter_AddRefs(mOptions));
  NS_ENSURE_SUCCESS(rv, rv);

  mLiveUpdate = history->GetUpdateRequirements(mQueries, mOptions,
                                               &mHasSearchTerms);
  return NS_OK;
}


// nsNavHistoryQueryResultNode::VerifyQueriesSerialized

nsresult
nsNavHistoryQueryResultNode::VerifyQueriesSerialized()
{
  if (! mURI.IsEmpty()) {
    return NS_OK;
  }
  NS_ASSERTION(mQueries.Count() > 0 && mOptions,
               "Query nodes must have either a URI or query/options");

  nsTArray<nsINavHistoryQuery*> flatQueries;
  flatQueries.SetCapacity(mQueries.Count());
  for (PRInt32 i = 0; i < mQueries.Count(); i ++)
    flatQueries.AppendElement(static_cast<nsINavHistoryQuery*>
                                         (mQueries.ObjectAt(i)));

  nsNavHistory* history = nsNavHistory::GetHistoryService();
  NS_ENSURE_TRUE(history, NS_ERROR_OUT_OF_MEMORY);

  nsresult rv = history->QueriesToQueryString(flatQueries.Elements(),
                                              flatQueries.Length(),
                                              mOptions, mURI);
  NS_ENSURE_SUCCESS(rv, rv);
  NS_ENSURE_TRUE(! mURI.IsEmpty(), NS_ERROR_FAILURE);
  return NS_OK;
}


// nsNavHistoryQueryResultNode::FillChildren

nsresult
nsNavHistoryQueryResultNode::FillChildren()
{
  NS_ASSERTION(! mContentsValid,
               "Don't call FillChildren when contents are valid");
  NS_ASSERTION(mChildren.Count() == 0,
               "We are trying to fill children when there already are some");

  nsNavHistory* history = nsNavHistory::GetHistoryService();
  NS_ENSURE_TRUE(history, NS_ERROR_OUT_OF_MEMORY);

  // get the results from the history service
  nsresult rv = VerifyQueriesParsed();
  NS_ENSURE_SUCCESS(rv, rv);
  rv = history->GetQueryResults(this, mQueries, mOptions, &mChildren);
  NS_ENSURE_SUCCESS(rv, rv);

  // it is important to call FillStats to fill in the parents on all
  // nodes and the result node pointers on the containers
  FillStats();

  PRUint16 sortType = GetSortType();

  // The default SORT_BY_NONE sorts by the bookmark index (position), 
  // which we do not have for history queries
  if (mOptions->QueryType() != nsINavHistoryQueryOptions::QUERY_TYPE_HISTORY ||
      sortType != nsINavHistoryQueryOptions::SORT_BY_NONE) {
    // once we've computed all tree stats, we can sort, because containers will
    // then have proper visit counts and dates
    SortComparator comparator = GetSortingComparator(GetSortType());
    if (comparator) {
      nsCAutoString sortingAnnotation;
      GetSortingAnnotation(sortingAnnotation);
      RecursiveSort(sortingAnnotation.get(), comparator);
    }
  }

  // if we are limiting our results remove items from the end of the
  // mChildren array after sorting. This is done for root node only.
  // note, if count < max results, we won't do anything.
  if (!mParent && mOptions->MaxResults()) {
    while (mChildren.Count() > mOptions->MaxResults())
      mChildren.RemoveObjectAt(mChildren.Count() - 1);
  }

  nsNavHistoryResult* result = GetResult();
  NS_ENSURE_TRUE(result, NS_ERROR_FAILURE);

  if (mOptions->QueryType() == nsINavHistoryQueryOptions::QUERY_TYPE_HISTORY ||
      mOptions->QueryType() == nsINavHistoryQueryOptions::QUERY_TYPE_UNIFIED) {
    // register with the result for history updates
    result->AddHistoryObserver(this);
  }

  if (mOptions->QueryType() == nsINavHistoryQueryOptions::QUERY_TYPE_BOOKMARKS ||
      mOptions->QueryType() == nsINavHistoryQueryOptions::QUERY_TYPE_UNIFIED ||
      mLiveUpdate == QUERYUPDATE_COMPLEX_WITH_BOOKMARKS) {
    // register with the result for bookmark updates
    result->AddAllBookmarksObserver(this);
  }

  mContentsValid = PR_TRUE;
  return NS_OK;
}


// nsNavHistoryQueryResultNode::ClearChildren
//
//    Call with unregister = false when we are going to update the children (for
//    example, when the container is open). This will clear the list and notify
//    all the children that they are going away.
//
//    When the results are becoming invalid and we are not going to refresh
//    them, set unregister = true, which will unregister the listener from the
//    result if any. We use unregister = false when we are refreshing the list
//    immediately so want to stay a notifier.

void
nsNavHistoryQueryResultNode::ClearChildren(PRBool aUnregister)
{
  for (PRInt32 i = 0; i < mChildren.Count(); i ++)
    mChildren[i]->OnRemoving();
  mChildren.Clear();

  if (aUnregister && mContentsValid) {
    nsNavHistoryResult* result = GetResult();
    if (result) {
      result->RemoveHistoryObserver(this);
      result->RemoveAllBookmarksObserver(this);
    }
  }
  mContentsValid = PR_FALSE;
}


// nsNavHistoryQueryResultNode::Refresh
//
//    This is called to update the result when something has changed that we
//    can not incrementally update.

nsresult
nsNavHistoryQueryResultNode::Refresh()
{
  // Ignore refreshes when there is a batch, EndUpdateBatch will do a refresh
  // to get all the changes.
  if (mBatchInProgress)
    return NS_OK;

  // This is not a root node but it does not have a parent - this means that 
  // the node has already been cleared and it is now called, because it was 
  // left in a local copy of the observers array.
  if (mIndentLevel > -1 && !mParent)
    return NS_OK;

  if (! mExpanded) {
    // when we are not expanded, we don't update, just invalidate and unhook
    ClearChildren(PR_TRUE);
    return NS_OK; // no updates in tree state
  }

  if (mLiveUpdate == QUERYUPDATE_COMPLEX_WITH_BOOKMARKS)
    ClearChildren(PR_TRUE);
  else
    ClearChildren(PR_FALSE);

  // ignore errors from FillChildren, since we will still want to refresh
  // the tree (there just might not be anything in it on error)
  (void)FillChildren();

  nsNavHistoryResult* result = GetResult();
  NS_ENSURE_TRUE(result, NS_ERROR_FAILURE);
  if (result->GetView())
    return result->GetView()->InvalidateContainer(
        static_cast<nsNavHistoryContainerResultNode*>(this));
  return NS_OK;
}


// nsNavHistoryQueryResultNode::GetSortType
//
//    Here, we override GetSortType to return the current sorting for this
//    query. GetSortType is used when dynamically inserting query results so we
//    can see which comparator we should use to find the proper insertion point
//    (it shouldn't be called from folder containers which maintain their own
//    sorting).
//
//    Normally, the container just forwards it up the chain. This is what
//    we want for host groups, for example. For queries, we often want to
//    use the query's sorting mode.
//
//    However, we only use this query node's sorting when it is not the root.
//    When it is the root, we use the result's sorting mode, which is set
//    according to the column headers in the tree view (if attached). This is
//    because there are two cases:
//
//    - You are looking at a bookmark hierarchy that contains an embedded
//      result. We should always use the query's sort ordering since the result
//      node's headers have nothing to do with us (and are disabled).
//
//    - You are looking at a query in the tree. In this case, we want the
//      result sorting to override ours (it should be initialized to the same
//      sorting mode).
//
//    It might seem like the column headers should set the sorting mode for the
//    query in the result so we don't have to have this special case. But, the
//    UI allows you to save the query in a different place or edit it, and just
//    grabs the parameters and options from the query node. It would be weird
//    to build a query, click a column header, and have the options you built
//    up in the query builder be changed from under you.

PRUint16
nsNavHistoryQueryResultNode::GetSortType()
{
  if (mParent) {
    // use our sorting, we are not the root
    return mOptions->SortingMode();
  }
  if (mResult) {
    return mResult->mSortingMode;
  }

  NS_NOTREACHED("We should always have a result");
  return nsINavHistoryQueryOptions::SORT_BY_NONE;
}

void
nsNavHistoryQueryResultNode::GetSortingAnnotation(nsACString& aAnnotation) {
  if (mParent) {
    // use our sorting, we are not the root
    mOptions->GetSortingAnnotation(aAnnotation);
  }
  else if (mResult) {
    aAnnotation.Assign(mResult->mSortingAnnotation);
  }
  else
    NS_NOTREACHED("We should always have a result");
}
// nsNavHistoryResultNode::OnBeginUpdateBatch

NS_IMETHODIMP
nsNavHistoryQueryResultNode::OnBeginUpdateBatch()
{
  mBatchInProgress = PR_TRUE;
  return NS_OK;
}


// nsNavHistoryResultNode::OnEndUpdateBatch
//
//    Always requery when batches are done. These will typically involve large
//    operations (currently delete) and it is likely more efficient to requery
//    than to incrementally update as each message comes in.

NS_IMETHODIMP
nsNavHistoryQueryResultNode::OnEndUpdateBatch()
{
  NS_ASSERTION(mBatchInProgress, "EndUpdateBatch without a begin");
  // must set to false before calling Refresh or Refresh will ignore us
  mBatchInProgress = PR_FALSE;
  return Refresh();
}


// nsNavHistoryQueryResultNode::OnVisit
//
//    Here we need to update all copies of the URI we have with the new visit
//    count, and potentially add a new entry in our query. This is the most
//    common update operation and it is important that it be as efficient as
//    possible.

NS_IMETHODIMP
nsNavHistoryQueryResultNode::OnVisit(nsIURI* aURI, PRInt64 aVisitId,
                                     PRTime aTime, PRInt64 aSessionId,
                                     PRInt64 aReferringId,
                                     PRUint32 aTransitionType,
                                     PRUint32* aAdded)
{
  // ignore everything during batches
  if (mBatchInProgress)
    return NS_OK;

  nsNavHistory* history = nsNavHistory::GetHistoryService();
  NS_ENSURE_TRUE(history, NS_ERROR_OUT_OF_MEMORY);

  nsresult rv;
  nsRefPtr<nsNavHistoryResultNode> addition;
  switch(mLiveUpdate) {

    case QUERYUPDATE_HOST: {
      // For these simple yet common cases we can check the host ourselves
      // before doing the overhead of creating a new result node.
      NS_ASSERTION(mQueries.Count() == 1, 
          "Host updated queries can have only one object");
      nsCOMPtr<nsNavHistoryQuery> queryHost = 
          do_QueryInterface(mQueries[0], &rv);
      NS_ENSURE_SUCCESS(rv, rv);

      PRBool hasDomain;
      queryHost->GetHasDomain(&hasDomain);
      if (!hasDomain || !queryHost->DomainIsHost())
        return NS_OK;

      nsCAutoString host;
      if (NS_FAILED(aURI->GetAsciiHost(host)))
        return NS_OK;

      if (!queryHost->Domain().Equals(host))
        return NS_OK;

    } // Let it fall through - we want to check the time too,
      // if the time is not present it will match too.
    case QUERYUPDATE_TIME: {
      // For these simple yet common cases we can check the time ourselves
      // before doing the overhead of creating a new result node.
      NS_ASSERTION(mQueries.Count() == 1, 
          "Time updated queries can have only one object");
      nsCOMPtr<nsNavHistoryQuery> query = 
          do_QueryInterface(mQueries[0], &rv);
      NS_ENSURE_SUCCESS(rv, rv);

      PRBool hasIt;
      query->GetHasBeginTime(&hasIt);
      if (hasIt) {
        PRTime beginTime = history->NormalizeTime(query->BeginTimeReference(),
                                                  query->BeginTime());
        if (aTime < beginTime)
          return NS_OK; // before our time range
      }
      query->GetHasEndTime(&hasIt);
      if (hasIt) {
        PRTime endTime = history->NormalizeTime(query->EndTimeReference(),
                                                query->EndTime());
        if (aTime > endTime)
          return NS_OK; // after our time range
      }
      // now we know that our visit satisfies the time range, create a new node
      rv = history->VisitIdToResultNode(aVisitId, mOptions,
                                        getter_AddRefs(addition));
      NS_ENSURE_SUCCESS(rv, rv);

      // We do not want to add this result to this node
      if (!addition)
          return NS_OK;

      break;
    }
    case QUERYUPDATE_SIMPLE: {
      // The history service can tell us whether the new item should appear
      // in the result. We first have to construct a node for it to check.
      rv = history->VisitIdToResultNode(aVisitId, mOptions,
                                        getter_AddRefs(addition));
      NS_ENSURE_SUCCESS(rv, rv);
      if (! history->EvaluateQueryForNode(mQueries, mOptions, addition))
        return NS_OK; // don't need to include in our query
      break;
    }
    case QUERYUPDATE_COMPLEX:
    case QUERYUPDATE_COMPLEX_WITH_BOOKMARKS:
      // need to requery in complex cases
      return Refresh();
    default:
      NS_NOTREACHED("Invalid value for mLiveUpdate");
      return Refresh();
  }

  // NOTE: The dynamic updating never deletes any nodes. Sometimes it replaces
  // URI nodes or adds visits, but never deletes old ones.
  //
  // The only time this might happen in the current implementation is if the
  // title changes and it no longer matches a keyword search. This is not
  // important enough to handle given that we don't do any other deleting. It
  // is arguably more useful behavior anyway, since you're searching your
  // history and the page used to match.
  //
  // When more queries are possible (show pages I've visited less than 5 times)
  // this will be important to add.

  nsCOMArray<nsNavHistoryResultNode> mergerNode;

  if (!mergerNode.AppendObject(addition))
    return NS_ERROR_OUT_OF_MEMORY;

  MergeResults(&mergerNode);

  if (aAdded)
    (*aAdded)++;

  return NS_OK;
}


// nsNavHistoryQueryResultNode::OnTitleChanged
//
//    Find every node that matches this URI and rename it. We try to do
//    incremental updates here, even when we are closed, because changing titles
//    is easier than requerying if we are invalid.
//
//    This actually gets called a lot. Typically, we will get an AddURI message
//    when the user visits the page, and then the title will be set asynchronously
//    when the title element of the page is parsed.

NS_IMETHODIMP
nsNavHistoryQueryResultNode::OnTitleChanged(nsIURI* aURI,
                                            const nsAString& aPageTitle)
{
  if (mBatchInProgress)
    return NS_OK; // ignore everything during batches
  if (! mExpanded) {
    // When we are not expanded, we don't update, just invalidate and unhook.
    // It would still be pretty easy to traverse the results and update the
    // titles, but when a title changes, its unlikely that it will be the only
    // thing. Therefore, we just give up.
    ClearChildren(PR_TRUE);
    return NS_OK; // no updates in tree state
  }

  // See if our queries have any keyword matching. In this case, we can't just
  // replace the title on the items, but must redo the query. This could be
  // handled more efficiently, but it is hard because keyword matching might
  // match anything, including the title and other things.
  if (mHasSearchTerms) {
    return Refresh();
  }

  // compute what the new title should be
  nsCAutoString newTitle = NS_ConvertUTF16toUTF8(aPageTitle);

  PRBool onlyOneEntry = (mOptions->ResultType() ==
                         nsINavHistoryQueryOptions::RESULTS_AS_URI);
  return ChangeTitles(aURI, newTitle, PR_TRUE, onlyOneEntry);
}


// nsNavHistoryQueryResultNode::OnDeleteURI
//
//    Here, we can always live update by just deleting all occurrences of
//    the given URI.

NS_IMETHODIMP
nsNavHistoryQueryResultNode::OnDeleteURI(nsIURI *aURI)
{
  PRBool onlyOneEntry = (mOptions->ResultType() ==
                         nsINavHistoryQueryOptions::RESULTS_AS_URI);
  nsCAutoString spec;
  nsresult rv = aURI->GetSpec(spec);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMArray<nsNavHistoryResultNode> matches;
  RecursiveFindURIs(onlyOneEntry, this, spec, &matches);
  if (matches.Count() == 0)
    return NS_OK; // not found

  for (PRInt32 i = 0; i < matches.Count(); i ++) {
    nsNavHistoryResultNode* node = matches[i];
    nsNavHistoryContainerResultNode* parent = node->mParent;
    // URI nodes should always have parents
    NS_ENSURE_TRUE(parent, NS_ERROR_UNEXPECTED);
    
    PRInt32 childIndex = parent->FindChild(node);
    NS_ASSERTION(childIndex >= 0, "Child not found in parent");
    parent->RemoveChildAt(childIndex);

    if (parent->mChildren.Count() == 0 && parent->IsQuery()) {
      // when query subcontainers (like hosts) get empty we should remove them
      // as well. Just append this to our list and it will get evaluated later
      // in the loop.
      matches.AppendObject(parent);
    }
  }
  return NS_OK;
}


// nsNavHistoryQueryResultNode::OnClearHistory

NS_IMETHODIMP
nsNavHistoryQueryResultNode::OnClearHistory()
{
  Refresh();
  return NS_OK;
}


// nsNavHistoryQueryResultNode::OnPageChanged
//

static void PR_CALLBACK setFaviconCallback(
   nsNavHistoryResultNode* aNode, void* aClosure)
{
  const nsCString* newFavicon = static_cast<nsCString*>(aClosure);
  aNode->mFaviconURI = *newFavicon;
}
NS_IMETHODIMP
nsNavHistoryQueryResultNode::OnPageChanged(nsIURI *aURI, PRUint32 aWhat,
                         const nsAString &aValue)
{
  nsNavHistoryResult* result = GetResult();
  NS_ENSURE_TRUE(result, NS_ERROR_FAILURE);

  nsCAutoString spec;
  nsresult rv = aURI->GetSpec(spec);
  NS_ENSURE_SUCCESS(rv, rv);

  switch (aWhat) {
    case nsINavHistoryObserver::ATTRIBUTE_FAVICON: {
      nsCString newFavicon = NS_ConvertUTF16toUTF8(aValue);
      PRBool onlyOneEntry = (mOptions->ResultType() ==
                             nsINavHistoryQueryOptions::RESULTS_AS_URI);
      UpdateURIs(PR_TRUE, onlyOneEntry, PR_FALSE, spec, setFaviconCallback,
                 &newFavicon);
      break;
    }
    default:
      NS_WARNING("Unknown page changed notification");
  }
  return NS_OK;
}


// nsNavHistoryQueryResultNode::OnPageExpired
//
//    Do nothing. Perhaps we want to handle this case. If so, add the call to
//    the result to enumerate the history observers.

NS_IMETHODIMP
nsNavHistoryQueryResultNode::OnPageExpired(nsIURI* aURI, PRTime aVisitTime,
                                           PRBool aWholeEntry)
{
  return NS_OK;
}


// nsNavHistoryQueryResultNode bookmark observers
//
//    These are the bookmark observer functions for query nodes. They listen
//    for bookmark events and refresh the results if we have any dependence on
//    the bookmark system.

NS_IMETHODIMP
nsNavHistoryQueryResultNode::OnItemAdded(PRInt64 aItemId, PRInt64 aFolder,
                                          PRInt32 aIndex)
{
  if (mLiveUpdate == QUERYUPDATE_COMPLEX_WITH_BOOKMARKS)
    return Refresh();
  return NS_OK;
}
NS_IMETHODIMP
nsNavHistoryQueryResultNode::OnItemRemoved(PRInt64 aItemId, PRInt64 aFolder,
                                            PRInt32 aIndex)
{
  if (mLiveUpdate == QUERYUPDATE_COMPLEX_WITH_BOOKMARKS)
    return Refresh();
  return NS_OK;
}
NS_IMETHODIMP
nsNavHistoryQueryResultNode::OnItemChanged(PRInt64 aItemId,
                                           const nsACString& aProperty,
                                           PRBool aIsAnnotationProperty,
                                           const nsACString& aValue)
{
  // history observers should not get OnItemChanged
  // but should get the corresponding history notifications instead
  // for bookmark queries, "all bookmark" observers should get OnItemChanged
  // for example, when a title of a bookmark change, we want that to refresh
  if (mLiveUpdate == QUERYUPDATE_COMPLEX_WITH_BOOKMARKS)
    return Refresh();
  else
    NS_WARNING("history observers should not get OnItemChanged, but should get the corresponding history notifications instead");
  return NS_OK;
}

NS_IMETHODIMP
nsNavHistoryQueryResultNode::OnItemVisited(PRInt64 aItemId,
                                           PRInt64 aVisitId, PRTime aTime)
{
  // for bookmark queries, "all bookmark" observer should get OnItemVisited
  // but it is ignored.
  if (mLiveUpdate != QUERYUPDATE_COMPLEX_WITH_BOOKMARKS)
    NS_NOTREACHED("history observers should not get OnItemVisited, but should get OnVisit instead");
  return NS_OK;
}
NS_IMETHODIMP
nsNavHistoryQueryResultNode::OnItemMoved(PRInt64 aFolder, PRInt64 aOldParent,
                                            PRInt32 aOldIndex, PRInt64 aNewParent,
                                            PRInt32 aNewIndex)
{
  if (mLiveUpdate == QUERYUPDATE_COMPLEX_WITH_BOOKMARKS)
    return Refresh();
  return NS_OK;
}

// nsNavHistoryFolderResultNode ************************************************
//
//    HOW DYNAMIC FOLDER UPDATING WORKS
//
//    When you create a result, it will automatically keep itself in sync with
//    stuff that happens in the system. For folder nodes, this means changes
//    to bookmarks.
//
//    A folder will fill its children "when necessary." This means it is being
//    opened or whether we need to see if it is empty for twisty drawing. It
//    will then register its ID with the main result object that owns it. This
//    result object will listen for all bookmark notifications and pass those
//    notifications to folder nodes that have registered for that specific
//    folder ID.
//
//    When a bookmark folder is closed, it will not clear its children. Instead,
//    it will keep them and also stay registered as a listener. This means that
//    you can more quickly re-open the same folder without doing any work. This
//    happens a lot for menus, and bookmarks don't change very often.
//
//    When a message comes in and the folder is open, we will do the correct
//    operations to keep ourselves in sync with the bookmark service. If the
//    folder is closed, we just clear our list to mark it as invalid and
//    unregister as a listener. This means we do not have to keep maintaining
//    an up-to-date list for the entire bookmark menu structure in every place
//    it is used.
//
//    There is an additional wrinkle. If the result is attached to a treeview,
//    and we invalidate but the folder itself is visible (its parent is open),
//    we will immediately get asked if we are full. The folder will then query
//    its children. Thus, the whole benefit of incremental updating is lost.
//    Therefore, if we are in a treeview AND our parent is visible, we will
//    keep incremental updating.

NS_IMPL_ISUPPORTS_INHERITED1(nsNavHistoryFolderResultNode,
                             nsNavHistoryContainerResultNode,
                             nsINavHistoryQueryResultNode)

nsNavHistoryFolderResultNode::nsNavHistoryFolderResultNode(
    const nsACString& aTitle, nsNavHistoryQueryOptions* aOptions,
    PRInt64 aFolderId, const nsACString& aDynamicContainerType) :
  nsNavHistoryContainerResultNode(EmptyCString(), aTitle, EmptyCString(),
                                  nsNavHistoryResultNode::RESULT_TYPE_FOLDER,
                                  PR_FALSE, aDynamicContainerType, aOptions),
  mContentsValid(PR_FALSE),
  mQueryItemId(-1)
{
  mItemId = aFolderId;
}


// nsNavHistoryFolderResultNode::OnRemoving
//
//    Here we do not want to call ContainerResultNode::OnRemoving since our own
//    ClearChildren will do the same thing and more (unregister the observers).
//    The base ResultNode::OnRemoving will clear some regular node stats, so it
//    is OK.

void
nsNavHistoryFolderResultNode::OnRemoving()
{
  nsNavHistoryResultNode::OnRemoving();
  ClearChildren(PR_TRUE);
}


// nsNavHistoryFolderResultNode::OpenContainer
//
//    See nsNavHistoryQueryResultNode::OpenContainer

nsresult
nsNavHistoryFolderResultNode::OpenContainer()
{
  NS_ASSERTION(! mExpanded, "Container must be expanded to close it");
  nsresult rv;

  if (! mContentsValid) {
    rv = FillChildren();
    NS_ENSURE_SUCCESS(rv, rv);
    if (IsDynamicContainer()) {
      // dynamic container API may want to change the bookmarks for this folder.
      nsCOMPtr<nsIDynamicContainer> svc = do_GetService(mDynamicContainerType.get(), &rv);
      if (NS_SUCCEEDED(rv)) {
        svc->OnContainerNodeOpening(
            static_cast<nsNavHistoryContainerResultNode*>(this), mOptions);
      } else {
        NS_WARNING("Unable to get dynamic container for ");
        NS_WARNING(mDynamicContainerType.get());
      }
    }
  }
  mExpanded = PR_TRUE;

  nsNavHistoryResult* result = GetResult();
  NS_ENSURE_TRUE(result, NS_ERROR_FAILURE);
  if (result->GetView())
    result->GetView()->ContainerOpened(
        static_cast<nsNavHistoryContainerResultNode*>(this));
  return NS_OK;
}


// nsNavHistoryFolderResultNode::GetHasChildren
//
//    See nsNavHistoryQueryResultNode::HasChildren.
//
//    The semantics here are a little different. Querying the contents of a
//    bookmark folder is relatively fast and it is common to have empty folders.
//    Therefore, we always want to return the correct result so that twisties
//    are drawn properly.

NS_IMETHODIMP
nsNavHistoryFolderResultNode::GetHasChildren(PRBool* aHasChildren)
{
  if (! mContentsValid) {
    nsresult rv = FillChildren();
    NS_ENSURE_SUCCESS(rv, rv);
  }
  *aHasChildren = (mChildren.Count() > 0);
  return NS_OK;
}

// nsNavHistoryFolderResultNode::GetItemId
//
// Returns the id of the item from which the folder node was generated, it
// could be either a concrete folder-itemId or the id used in
// a simple-folder-query-bookmark (place:folder=X)

NS_IMETHODIMP
nsNavHistoryFolderResultNode::GetItemId(PRInt64* aItemId)
{
  *aItemId = mQueryItemId == -1 ? mItemId : mQueryItemId;
  return NS_OK;
}

// nsNavHistoryFolderResultNode::GetChildrenReadOnly
//
//    Here, we override the getter and ignore the value stored in our object.
//    The bookmarks service can tell us whether this folder should be read-only
//    or not.
//
//    It would be nice to put this code in the folder constructor, but the
//    database was complaining. I believe it is because most folders are
//    created while enumerating the bookmarks table and having a statement
//    open, and doing another statement might make it unhappy in some cases.

NS_IMETHODIMP
nsNavHistoryFolderResultNode::GetChildrenReadOnly(PRBool *aChildrenReadOnly)
{
  nsNavBookmarks* bookmarks = nsNavBookmarks::GetBookmarksService();
  NS_ENSURE_TRUE(bookmarks, NS_ERROR_UNEXPECTED);
  return bookmarks->GetFolderReadonly(mItemId, aChildrenReadOnly);
}


// nsNavHistoryFolderResultNode::GetFolderItemId

NS_IMETHODIMP
nsNavHistoryFolderResultNode::GetFolderItemId(PRInt64* aItemId)
{
  *aItemId = mItemId;
  return NS_OK;
}

// nsNavHistoryFolderResultNode::GetUri
//
//    This lazily computes the URI for this specific folder query with
//    the current options.

NS_IMETHODIMP
nsNavHistoryFolderResultNode::GetUri(nsACString& aURI)
{
  if (! mURI.IsEmpty()) {
    aURI = mURI;
    return NS_OK;
  }

  PRUint32 queryCount;
  nsINavHistoryQuery** queries;
  nsresult rv = GetQueries(&queryCount, &queries);
  NS_ENSURE_SUCCESS(rv, rv);

  nsNavHistory* history = nsNavHistory::GetHistoryService();
  NS_ENSURE_TRUE(history, NS_ERROR_OUT_OF_MEMORY);

  rv = history->QueriesToQueryString(queries, queryCount, mOptions, aURI);
  for (PRUint32 queryIndex = 0; queryIndex < queryCount;  queryIndex ++) {
    NS_RELEASE(queries[queryIndex]);
  }
  nsMemory::Free(queries);
  return rv;
}


// nsNavHistoryFolderResultNode::GetQueries
//
//    This just returns the queries that give you this bookmarks folder

NS_IMETHODIMP
nsNavHistoryFolderResultNode::GetQueries(PRUint32* queryCount,
                                         nsINavHistoryQuery*** queries)
{
  // get the query object
  nsCOMPtr<nsINavHistoryQuery> query;
  nsNavHistory* history = nsNavHistory::GetHistoryService();
  NS_ENSURE_TRUE(history, NS_ERROR_OUT_OF_MEMORY);
  nsresult rv = history->GetNewQuery(getter_AddRefs(query));
  NS_ENSURE_SUCCESS(rv, rv);

  // query just has the folder ID set and nothing else
  rv = query->SetFolders(&mItemId, 1);
  NS_ENSURE_SUCCESS(rv, rv);

  // make array of our 1 query
  *queries = static_cast<nsINavHistoryQuery**>
                        (nsMemory::Alloc(sizeof(nsINavHistoryQuery*)));
  if (! *queries)
    return NS_ERROR_OUT_OF_MEMORY;
  NS_ADDREF((*queries)[0] = query);
  *queryCount = 1;
  return NS_OK;
}


// nsNavHistoryFolderResultNode::GetQueryOptions
//
//    Options for the query that gives you this bookmarks folder. This is just
//    the options for the folder with the current folder ID set.

NS_IMETHODIMP
nsNavHistoryFolderResultNode::GetQueryOptions(
                                      nsINavHistoryQueryOptions** aQueryOptions)
{
  NS_ASSERTION(mOptions, "Options invalid");

  *aQueryOptions = mOptions;
  NS_ADDREF(*aQueryOptions);
  return NS_OK;
}


// nsNavHistoryFolderResultNode::FillChildren
//
//    Call to fill the actual children of this folder.

nsresult
nsNavHistoryFolderResultNode::FillChildren()
{
  NS_ASSERTION(! mContentsValid,
               "Don't call FillChildren when contents are valid");
  NS_ASSERTION(mChildren.Count() == 0,
               "We are trying to fill children when there already are some");

  nsNavBookmarks* bookmarks = nsNavBookmarks::GetBookmarksService();
  NS_ENSURE_TRUE(bookmarks, NS_ERROR_OUT_OF_MEMORY);

  // actually get the folder children from the bookmark service
  nsresult rv = bookmarks->QueryFolderChildren(mItemId, mOptions, &mChildren);
  NS_ENSURE_SUCCESS(rv, rv);

  // PERFORMANCE: it may be better to also fill any child folders at this point
  // so that we can draw tree twisties without doing a separate query later. If
  // we don't end up drawing twisties a lot, it doesn't matter. If we do this,
  // we should wrap everything in a transaction here on the bookmark service's
  // connection.

  // it is important to call FillStats to fill in the parents on all
  // nodes and the result node pointers on the containers
  FillStats();

  // once we've computed all tree stats, we can sort, because containers will
  // then have proper visit counts and dates
  SortComparator comparator = GetSortingComparator(GetSortType());
  if (comparator) {
    nsCAutoString sortingAnnotation;
    GetSortingAnnotation(sortingAnnotation);
    RecursiveSort(sortingAnnotation.get(), comparator);
  }

  // if we are limiting our results remove items from the end of the
  // mChildren array after sorting. This is done for root node only.
  // note, if count < max results, we won't do anything.
  if (!mParent && mOptions->MaxResults()) {
    while (mChildren.Count() > mOptions->MaxResults())
      mChildren.RemoveObjectAt(mChildren.Count() - 1);
  }

  // register with the result for updates
  nsNavHistoryResult* result = GetResult();
  NS_ENSURE_TRUE(result, NS_ERROR_FAILURE);
  result->AddBookmarkFolderObserver(this, mItemId);

  mContentsValid = PR_TRUE;
  return NS_OK;
}


// nsNavHistoryFolderResultNode::ClearChildren
//
//    See nsNavHistoryQueryResultNode::FillChildren

void
nsNavHistoryFolderResultNode::ClearChildren(PRBool unregister)
{
  for (PRInt32 i = 0; i < mChildren.Count(); i ++)
    mChildren[i]->OnRemoving();
  mChildren.Clear();

  if (unregister && mContentsValid) {
    nsNavHistoryResult* result = GetResult();
    if (result)
      result->RemoveBookmarkFolderObserver(this, mItemId);
  }
  mContentsValid = PR_FALSE;
}


// nsNavHistoryFolderResultNode::Refresh
//
//    This is called to update the result when something has changed that we
//    can not incrementally update.

nsresult
nsNavHistoryFolderResultNode::Refresh()
{
  ClearChildren(PR_TRUE);

  if (! mExpanded) {
    // when we are not expanded, we don't update, just invalidate and unhook
    return NS_OK; // no updates in tree state
  }

  // ignore errors from FillChildren, since we will still want to refresh
  // the tree (there just might not be anything in it on error). Need to
  // unregister as an observer because FillChildren will try to re-register us.
  (void)FillChildren();

  nsNavHistoryResult* result = GetResult();
  NS_ENSURE_TRUE(result, NS_ERROR_FAILURE);
  if (result->GetView())
    return result->GetView()->InvalidateContainer(
        static_cast<nsNavHistoryContainerResultNode*>(this));
  return NS_OK;
}


// nsNavHistoryFolderResultNode::StartIncrementalUpdate
//
//    This implements the logic described above the constructor. This sees if
//    we should do an incremental update and returns true if so. If not, it
//    invalidates our children, unregisters us an observer, and returns false.

PRBool
nsNavHistoryFolderResultNode::StartIncrementalUpdate()
{
  // if any items are excluded, we can not do incremental updates since the
  // indices from the bookmark service will not be valid
  nsCAutoString parentAnnotationToExclude;
  nsresult rv = mOptions->GetExcludeItemIfParentHasAnnotation(parentAnnotationToExclude);
  NS_ENSURE_SUCCESS(rv, PR_FALSE);

  if (! mOptions->ExcludeItems() && 
      ! mOptions->ExcludeQueries() && 
      ! mOptions->ExcludeReadOnlyFolders() && 
      parentAnnotationToExclude.IsEmpty()) {

    // easy case: we are visible, always do incremental update
    if (mExpanded || AreChildrenVisible())
      return PR_TRUE;

    nsNavHistoryResult* result = GetResult();
    NS_ENSURE_TRUE(result, PR_FALSE);

    // when a tree is attached also do incremental updates if our parent is
    // visible so that twisties are drawn correctly.
    if (mParent && result->GetView())
      return PR_TRUE;
  }

  // otherwise, we don't do incremental updates, invalidate and unregister
  Refresh();
  return PR_FALSE;
}


// nsNavHistoryFolderResultNode::ReindexRange
//
//    This function adds aDelta to all bookmark indices between the two
//    endpoints, inclusive. It is used when items are added or removed from
//    the bookmark folder.

void
nsNavHistoryFolderResultNode::ReindexRange(PRInt32 aStartIndex,
                                           PRInt32 aEndIndex,
                                           PRInt32 aDelta)
{
  for (PRInt32 i = 0; i < mChildren.Count(); i ++) {
    nsNavHistoryResultNode* node = mChildren[i];
    if (node->mBookmarkIndex >= aStartIndex &&
        node->mBookmarkIndex <= aEndIndex)
      node->mBookmarkIndex += aDelta;
  }
}


// nsNavHistoryFolderResultNode::FindChildById
//
//    Searches this folder for a node with the given URI. Returns null if not
//    found. Does not addref the node!

nsNavHistoryResultNode*
nsNavHistoryFolderResultNode::FindChildById(PRInt64 aItemId,
    PRUint32* aNodeIndex)
{
  for (PRInt32 i = 0; i < mChildren.Count(); i ++) {
    if (mChildren[i]->mItemId == aItemId) {
      *aNodeIndex = i;
      return mChildren[i];
    }
  }
  return nsnull;
}


// nsNavHistoryFolderResultNode::OnBeginUpdateBatch (nsINavBookmarkObserver)

NS_IMETHODIMP
nsNavHistoryFolderResultNode::OnBeginUpdateBatch()
{
  return NS_OK;
}


// nsNavHistoryFolderResultNode::OnEndUpdateBatch (nsINavBookmarkObserver)

NS_IMETHODIMP
nsNavHistoryFolderResultNode::OnEndUpdateBatch()
{
  return NS_OK;
}


// nsNavHistoryFolderResultNode::OnItemAdded (nsINavBookmarkObserver)

NS_IMETHODIMP
nsNavHistoryFolderResultNode::OnItemAdded(PRInt64 aItemId,
                                          PRInt64 aParentFolder,
                                          PRInt32 aIndex)
{
  NS_ASSERTION(aParentFolder == mItemId, "Got wrong bookmark update");

  // here, try to do something reasonable if the bookmark service gives us
  // a bogus index.
  if (aIndex < 0) {
    NS_NOTREACHED("Invalid index for item adding: <0");
    aIndex = 0;
  } else if (aIndex > mChildren.Count()) {
    NS_NOTREACHED("Invalid index for item adding: greater than count");
    aIndex = mChildren.Count();
  }

  nsNavBookmarks* bookmarks = nsNavBookmarks::GetBookmarksService();
  NS_ENSURE_TRUE(bookmarks, NS_ERROR_OUT_OF_MEMORY);

  PRUint16 itemType;
  nsresult rv = bookmarks->GetItemType(aItemId, &itemType);
  NS_ENSURE_SUCCESS(rv, rv);

  // check for query URIs, which are bookmarks, but treated as containers
  // in results and views.
  PRBool isQuery = PR_FALSE;
  if (itemType == nsINavBookmarksService::TYPE_BOOKMARK) {
    nsCOMPtr<nsIURI> itemURI;
    rv = bookmarks->GetBookmarkURI(aItemId, getter_AddRefs(itemURI));
    NS_ENSURE_SUCCESS(rv, rv);
    nsCAutoString itemURISpec;
    rv = itemURI->GetSpec(itemURISpec);
    NS_ENSURE_SUCCESS(rv, rv);
    isQuery = IsQueryURI(itemURISpec);
  }

  if (itemType != nsINavBookmarksService::TYPE_FOLDER &&
      !isQuery && mOptions->ExcludeItems()) {
    // don't update items when we aren't displaying them, but we still need
    // to adjust bookmark indices to account for the insertion
    ReindexRange(aIndex, PR_INT32_MAX, 1);
    return NS_OK; 
  }

  if (!StartIncrementalUpdate())
    return NS_OK; // folder was completely refreshed for us

  // adjust indices to account for insertion
  ReindexRange(aIndex, PR_INT32_MAX, 1);

  nsRefPtr<nsNavHistoryResultNode> node;
  if (itemType == nsINavBookmarksService::TYPE_BOOKMARK) {
    nsNavHistory* history = nsNavHistory::GetHistoryService();
    NS_ENSURE_TRUE(history, NS_ERROR_OUT_OF_MEMORY);
    rv = history->BookmarkIdToResultNode(aItemId, mOptions, getter_AddRefs(node));
    NS_ENSURE_SUCCESS(rv, rv);
    node->mItemId = aItemId;
  }
  else if (itemType == nsINavBookmarksService::TYPE_FOLDER) {
    rv = bookmarks->ResultNodeForContainer(aItemId, mOptions, getter_AddRefs(node));
    NS_ENSURE_SUCCESS(rv, rv);
  }
  else if (itemType == nsINavBookmarksService::TYPE_SEPARATOR) {
    node = new nsNavHistorySeparatorResultNode();
    NS_ENSURE_TRUE(node, NS_ERROR_OUT_OF_MEMORY);
    node->mItemId = aItemId;
  }
  node->mBookmarkIndex = aIndex;

  if (itemType == nsINavBookmarksService::TYPE_SEPARATOR ||
      GetSortType() == nsINavHistoryQueryOptions::SORT_BY_NONE) {
    // insert at natural bookmarks position
    return InsertChildAt(node, aIndex);
  }
  // insert at sorted position
  return InsertSortedChild(node, PR_FALSE);
}


// nsNavHistoryFolderResultNode::OnItemRemoved (nsINavBookmarkObserver)

NS_IMETHODIMP
nsNavHistoryFolderResultNode::OnItemRemoved(PRInt64 aItemId,
                                            PRInt64 aParentFolder, PRInt32 aIndex)
{
  // We only care about notifications when a child changes. When the deleted
  // item is us, our parent should also be registered and will remove us from
  // its list.
  if (mItemId == aItemId)
    return NS_OK;

  // don't trust the index from the bookmark service, find it ourselves. The
  // sorting could be different, or the bookmark services indices and ours might
  // be out of sync somehow.
  PRUint32 index;
  nsNavHistoryResultNode* node = FindChildById(aItemId, &index);
  if (!node) {
    NS_NOTREACHED("Removing item we don't have");
    return NS_ERROR_FAILURE;
  }

  NS_ASSERTION(aParentFolder == mItemId, "Got wrong bookmark update");

  if ((node->IsURI() || node->IsSeparator()) && mOptions->ExcludeItems()) {
    // don't update items when we aren't displaying them, but we do need to
    // adjust everybody's bookmark indices to account for the removal
    ReindexRange(aIndex, PR_INT32_MAX, -1);
    return NS_OK;
  }

  if (!StartIncrementalUpdate())
    return NS_OK; // we are completely refreshed

  // shift all following indices down
  ReindexRange(aIndex + 1, PR_INT32_MAX, -1);

  return RemoveChildAt(index);
}


// nsNavHistoryResultNode::OnItemChanged

NS_IMETHODIMP
nsNavHistoryResultNode::OnItemChanged(PRInt64 aItemId,
                                      const nsACString& aProperty,
                                      PRBool aIsAnnotationProperty,
                                      const nsACString& aValue)
{
  if (aProperty.EqualsLiteral("title")) {
    // XXX: what should we do if the new title is void?
    mTitle = aValue;
  }
  else if (aProperty.EqualsLiteral("uri")) {
    mURI = aValue;
    // clear the tags string as well
    mTags.SetIsVoid(PR_TRUE);
  }
  else if (aProperty.EqualsLiteral("favicon")) {
    mFaviconURI = aValue;
  }
  else if (aProperty.EqualsLiteral("cleartime")) {
    mTime = 0;
  }
  else if (aProperty.EqualsLiteral("tags")) {
    mTags.SetIsVoid(PR_TRUE);
  }
  else if (!aProperty.EqualsLiteral("keyword") && !aIsAnnotationProperty) {
    // XXX: expose a keyword getter on bookmarks nodes?

    NS_NOTREACHED("Unknown bookmark property changing.");
  }

  nsNavBookmarks* bookmarks = nsNavBookmarks::GetBookmarksService();
  NS_ENSURE_TRUE(bookmarks, NS_ERROR_UNEXPECTED);

  PRTime lastModified;
  nsresult rv = bookmarks->GetItemLastModified(aItemId, &lastModified);
  if (NS_SUCCEEDED(rv)) {
    mLastModified = lastModified;
  }
  else {
    mLastModified = 0;
  }

  PRTime dateAdded;
  rv = bookmarks->GetItemDateAdded(aItemId, &dateAdded);
  if (NS_SUCCEEDED(rv)) {
    mDateAdded = dateAdded;
  }
  else {
    mDateAdded = 0;
  }

  nsNavHistoryResult* result = GetResult();
  NS_ENSURE_TRUE(result, NS_ERROR_FAILURE);

  if (result->GetView() && (!mParent || mParent->AreChildrenVisible())) {
    result->GetView()->ItemChanged(this);
  }

  if (!mParent)
    return NS_OK;

  // DO NOT OPTIMIZE THIS TO CHECK aProperty
  // the sorting methods fall back to each other so we need to re-sort the
  // result even if it's not set to sort by the given property
  PRInt32 ourIndex = mParent->FindChild(this);
  mParent->EnsureItemPosition(ourIndex);

  return NS_OK;
}

NS_IMETHODIMP
nsNavHistoryFolderResultNode::OnItemChanged(PRInt64 aItemId,
                                            const nsACString& aProperty,
                                            PRBool aIsAnnotationProperty,
                                            const nsACString& aValue) {
  // The query-item's title is used for simple-query nodes
  if (mQueryItemId != -1) {
    PRBool isTitleChange = aProperty.EqualsLiteral("title");
    if ((mQueryItemId == aItemId && !isTitleChange) ||
        (mQueryItemId != aItemId && isTitleChange)) {
      return NS_OK;
    }
  }

  return nsNavHistoryResultNode::OnItemChanged(aItemId, aProperty,
                                               aIsAnnotationProperty,
                                               aValue);
}

// nsNavHistoryFolderResultNode::OnItemVisited (nsINavBookmarkObserver)
//
//    Update visit count and last visit time and refresh.

NS_IMETHODIMP
nsNavHistoryFolderResultNode::OnItemVisited(PRInt64 aItemId,
                                            PRInt64 aVisitId, PRTime aTime)
{
  if (mOptions->ExcludeItems())
    return NS_OK; // don't update items when we aren't displaying them
  if (! StartIncrementalUpdate())
    return NS_OK;

  PRUint32 nodeIndex;
  nsNavHistoryResultNode* node = FindChildById(aItemId, &nodeIndex);
  if (! node)
    return NS_ERROR_FAILURE;

  nsNavHistoryResult* result = GetResult();
  NS_ENSURE_TRUE(result, NS_ERROR_FAILURE);

  // update node
  node->mTime = aTime;
  node->mAccessCount ++;

  // update us
  PRInt32 oldAccessCount = mAccessCount;
  mAccessCount ++;
  if (aTime > mTime)
    mTime = aTime;
  ReverseUpdateStats(mAccessCount - oldAccessCount);

  // update sorting if necessary
  PRUint32 sortType = GetSortType();
  if (sortType == nsINavHistoryQueryOptions::SORT_BY_VISITCOUNT_ASCENDING ||
      sortType == nsINavHistoryQueryOptions::SORT_BY_VISITCOUNT_DESCENDING ||
      sortType == nsINavHistoryQueryOptions::SORT_BY_DATE_ASCENDING ||
      sortType == nsINavHistoryQueryOptions::SORT_BY_DATE_DESCENDING) {
    PRInt32 childIndex = FindChild(node);
    NS_ASSERTION(childIndex >= 0, "Could not find child we just got a reference to");
    if (childIndex >= 0) {
      EnsureItemPosition(childIndex);
    }
  } else if (result->GetView() && AreChildrenVisible()) {
    // no sorting changed, just redraw the row if visible
    result->GetView()->ItemChanged(node);
  }
  return NS_OK;
}

// nsNavHistoryFolderResultNode::OnItemMoved (nsINavBookmarkObserver)

NS_IMETHODIMP
nsNavHistoryFolderResultNode::OnItemMoved(PRInt64 aItemId, PRInt64 aOldParent,
                                          PRInt32 aOldIndex, PRInt64 aNewParent,
                                          PRInt32 aNewIndex)
{
  NS_ASSERTION(aOldParent == mItemId || aNewParent == mItemId,
               "Got a bookmark message that doesn't belong to us");
  if (! StartIncrementalUpdate())
    return NS_OK; // entire container was refreshed for us

  if (aOldParent == aNewParent) {
    // getting moved within the same folder, we don't want to do a remove and
    // an add because that will lose your tree state.

    // adjust bookmark indices
    ReindexRange(aOldIndex + 1, PR_INT32_MAX, -1);
    ReindexRange(aNewIndex, PR_INT32_MAX, 1);

    PRUint32 index;
    nsNavHistoryResultNode* node = FindChildById(aItemId, &index);
    if (!node) {
      NS_NOTREACHED("Can't find folder that is moving!");
      return NS_ERROR_FAILURE;
    }
    NS_ASSERTION(index >= 0 && index < PRUint32(mChildren.Count()),
                 "Invalid index!");
    node->mBookmarkIndex = aNewIndex;

    // adjust position
    EnsureItemPosition(index);
    return NS_OK;
  } else {
    // moving between two different folders, just do a remove and an add
    if (aOldParent == mItemId)
      OnItemRemoved(aItemId, aOldParent, aOldIndex);
    if (aNewParent == mItemId)
      OnItemAdded(aItemId, aNewParent, aNewIndex);
  }
  return NS_OK;
}


// nsNavHistorySeparatorResultNode
//
// Separator nodes do not hold any data

nsNavHistorySeparatorResultNode::nsNavHistorySeparatorResultNode()
  : nsNavHistoryResultNode(EmptyCString(), EmptyCString(),
                           0, 0, EmptyCString())
{
}


// nsNavHistoryResult **********************************************************
NS_IMPL_CYCLE_COLLECTION_CLASS(nsNavHistoryResult)

NS_IMPL_CYCLE_COLLECTION_UNLINK_BEGIN(nsNavHistoryResult)
  NS_IMPL_CYCLE_COLLECTION_UNLINK_NSCOMPTR(mRootNode)
NS_IMPL_CYCLE_COLLECTION_UNLINK_END 

NS_IMPL_CYCLE_COLLECTION_TRAVERSE_BEGIN(nsNavHistoryResult)
  NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSCOMPTR_AMBIGUOUS(mRootNode, nsINavHistoryContainerResultNode)
NS_IMPL_CYCLE_COLLECTION_TRAVERSE_END

NS_IMPL_CYCLE_COLLECTING_ADDREF(nsNavHistoryResult)
NS_IMPL_CYCLE_COLLECTING_RELEASE(nsNavHistoryResult)

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(nsNavHistoryResult)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsINavHistoryResult)
  NS_INTERFACE_MAP_STATIC_AMBIGUOUS(nsNavHistoryResult)
  NS_INTERFACE_MAP_ENTRY(nsINavHistoryResult)
  NS_INTERFACE_MAP_ENTRY(nsINavBookmarkObserver)
  NS_INTERFACE_MAP_ENTRY(nsINavHistoryObserver)
  NS_INTERFACE_MAP_ENTRY(nsISupportsWeakReference)
NS_INTERFACE_MAP_END

nsNavHistoryResult::nsNavHistoryResult(nsNavHistoryContainerResultNode* aRoot) :
  mRootNode(aRoot),
  mIsHistoryObserver(PR_FALSE),
  mIsBookmarkFolderObserver(PR_FALSE),
  mIsAllBookmarksObserver(PR_FALSE),
  mBatchInProgress(PR_FALSE)
{
  mRootNode->mResult = this;
}

PR_STATIC_CALLBACK(PLDHashOperator)
RemoveBookmarkFolderObserversCallback(nsTrimInt64HashKey::KeyType aKey,
                                      nsNavHistoryResult::FolderObserverList*& aData,
                                      void* userArg)
{
  delete aData;
  return PL_DHASH_REMOVE;
}

nsNavHistoryResult::~nsNavHistoryResult()
{
  // delete all bookmark folder observer arrays which are allocated on the heap
  mBookmarkFolderObservers.Enumerate(&RemoveBookmarkFolderObserversCallback, nsnull);
}


// nsNavHistoryResult::Init
//
//    Call AddRef before this, since we may do things like register ourselves.

nsresult
nsNavHistoryResult::Init(nsINavHistoryQuery** aQueries,
                         PRUint32 aQueryCount,
                         nsNavHistoryQueryOptions *aOptions)
{
  nsresult rv;
  NS_ASSERTION(aOptions, "Must have valid options");
  NS_ASSERTION(aQueries && aQueryCount > 0, "Must have >1 query in result");

  // Fill saved source queries with copies of the original (the caller might
  // change their original objects, and we always want to reflect the source
  // parameters).
  for (PRUint32 i = 0; i < aQueryCount; i ++) {
    nsCOMPtr<nsINavHistoryQuery> queryClone;
    rv = aQueries[i]->Clone(getter_AddRefs(queryClone));
    NS_ENSURE_SUCCESS(rv, rv);
    if (!mQueries.AppendObject(queryClone))
      return NS_ERROR_OUT_OF_MEMORY;
  }
  rv = aOptions->Clone(getter_AddRefs(mOptions));
  NS_ENSURE_SUCCESS(rv, rv);
  mSortingMode = aOptions->SortingMode();
  rv = aOptions->GetSortingAnnotation(mSortingAnnotation);
  NS_ENSURE_SUCCESS(rv, rv);

  mPropertyBags.Init();
  if (! mBookmarkFolderObservers.Init(128))
    return NS_ERROR_OUT_OF_MEMORY;

  NS_ASSERTION(mRootNode->mIndentLevel == -1,
               "Root node's indent level initialized wrong");
  mRootNode->FillStats();

  return NS_OK;
}


// nsNavHistoryResult::NewHistoryResult
//
//    Constructs a new history result object.

nsresult // static
nsNavHistoryResult::NewHistoryResult(nsINavHistoryQuery** aQueries,
                                     PRUint32 aQueryCount,
                                     nsNavHistoryQueryOptions* aOptions,
                                     nsNavHistoryContainerResultNode* aRoot,
                                     nsNavHistoryResult** result)
{
  *result = new nsNavHistoryResult(aRoot);
  if (! *result)
    return NS_ERROR_OUT_OF_MEMORY;
  NS_ADDREF(*result); // must happen before Init
  nsresult rv = (*result)->Init(aQueries, aQueryCount, aOptions);
  if (NS_FAILED(rv)) {
    NS_RELEASE(*result);
    *result = nsnull;
    return rv;
  }
  return NS_OK;
}


// nsNavHistoryResult::PropertyBagFor
//
//    Given a pointer to a result node, this will give you the property bag
//    corresponding to it. Each node exposes a property bag to be used to
//    store temporary data. It is designed primarily for those implementing
//    container sources for storing data they need.
//
//    Since we expect very few result nodes will ever have their property bags
//    used, and since we can have a LOT of result nodes, we store the property
//    bags separately in a hash table in the parent result.
//
//    This function is called by a node when somebody wants the property bag.
//    It will create a property bag if necessary and store it for later
//    retrieval.

nsresult
nsNavHistoryResult::PropertyBagFor(nsISupports* aObject,
                                   nsIWritablePropertyBag** aBag)
{
  *aBag = nsnull;
  if (mPropertyBags.Get(aObject, aBag) && *aBag)
    return NS_OK;

  nsresult rv = NS_NewHashPropertyBag(aBag);
  NS_ENSURE_SUCCESS(rv, rv);
  if (! mPropertyBags.Put(aObject, *aBag)) {
    NS_RELEASE(*aBag);
    *aBag = nsnull;
    return NS_ERROR_OUT_OF_MEMORY;
  }
  return NS_OK;
}

// nsNavHistoryResult::AddHistoryObserver

void
nsNavHistoryResult::AddHistoryObserver(nsNavHistoryQueryResultNode* aNode)
{
  if (! mIsHistoryObserver) {
      nsNavHistory* history = nsNavHistory::GetHistoryService();
      NS_ASSERTION(history, "Can't create history service");
      history->AddObserver(this, PR_TRUE);
      mIsHistoryObserver = PR_TRUE;
  }
  if (mHistoryObservers.IndexOf(aNode) != mHistoryObservers.NoIndex) {
    NS_WARNING("Attempting to register an observer twice!");
    return;
  }
  mHistoryObservers.AppendElement(aNode);
}

// nsNavHistoryResult::AddAllBookmarksObserver

void
nsNavHistoryResult::AddAllBookmarksObserver(nsNavHistoryQueryResultNode* aNode)
{
  if (! mIsAllBookmarksObserver) {
    nsNavBookmarks* bookmarks = nsNavBookmarks::GetBookmarksService();
    if (! bookmarks) {
      NS_NOTREACHED("Can't create bookmark service");
      return;
    }
    bookmarks->AddObserver(this, PR_TRUE);
    mIsAllBookmarksObserver = PR_TRUE;
  }
  if (mAllBookmarksObservers.IndexOf(aNode) != mAllBookmarksObservers.NoIndex) {
    NS_WARNING("Attempting to register an observer twice!");
    return;
  }
  mAllBookmarksObservers.AppendElement(aNode);
}


// nsNavHistoryResult::AddBookmarkFolderObserver
//
//    Here, we also attach as a bookmark service observer if necessary

void
nsNavHistoryResult::AddBookmarkFolderObserver(nsNavHistoryFolderResultNode* aNode,
                                        PRInt64 aFolder)
{
  if (! mIsBookmarkFolderObserver) {
    nsNavBookmarks* bookmarks = nsNavBookmarks::GetBookmarksService();
    if (! bookmarks) {
      NS_NOTREACHED("Can't create bookmark service");
      return;
    }
    bookmarks->AddObserver(this, PR_TRUE);
    mIsBookmarkFolderObserver = PR_TRUE;
  }

  FolderObserverList* list = BookmarkFolderObserversForId(aFolder, PR_TRUE);
  if (list->IndexOf(aNode) != list->NoIndex) {
    NS_NOTREACHED("Attempting to register an observer twice!");
    return;
  }
  list->AppendElement(aNode);
}


// nsNavHistoryResult::RemoveHistoryObserver

void
nsNavHistoryResult::RemoveHistoryObserver(nsNavHistoryQueryResultNode* aNode)
{
  mHistoryObservers.RemoveElement(aNode);
}

// nsNavHistoryResult::RemoveAllBookmarksObserver

void
nsNavHistoryResult::RemoveAllBookmarksObserver(nsNavHistoryQueryResultNode* aNode)
{
  mAllBookmarksObservers.RemoveElement(aNode);
}


// nsNavHistoryResult::RemoveBookmarkFolderObserver

void
nsNavHistoryResult::RemoveBookmarkFolderObserver(nsNavHistoryFolderResultNode* aNode,
                                                 PRInt64 aFolder)
{
  FolderObserverList* list = BookmarkFolderObserversForId(aFolder, PR_FALSE);
  if (! list)
    return; // we don't even have an entry for that folder
  list->RemoveElement(aNode);
}


// nsNavHistoryResult::BookmarkFolderObserversForId

nsNavHistoryResult::FolderObserverList*
nsNavHistoryResult::BookmarkFolderObserversForId(PRInt64 aFolderId, PRBool aCreate)
{
  FolderObserverList* list;
  if (mBookmarkFolderObservers.Get(aFolderId, &list))
    return list;
  if (! aCreate)
    return nsnull;

  // need to create a new list
  list = new FolderObserverList;
  mBookmarkFolderObservers.Put(aFolderId, list);
  return list;
}

// nsNavHistoryResult::GetSortingMode (nsINavHistoryResult)

NS_IMETHODIMP
nsNavHistoryResult::GetSortingMode(PRUint16* aSortingMode)
{
  *aSortingMode = mSortingMode;
  return NS_OK;
}

// nsNavHistoryResult::SetSortingMode (nsINavHistoryResult)

NS_IMETHODIMP
nsNavHistoryResult::SetSortingMode(PRUint16 aSortingMode)
{
  if (aSortingMode > nsINavHistoryQueryOptions::SORT_BY_ANNOTATION_DESCENDING)
    return NS_ERROR_INVALID_ARG;
  if (! mRootNode)
    return NS_ERROR_FAILURE;

  // keep everything in sync
  NS_ASSERTION(mOptions, "Options should always be present for a root query");

  mSortingMode = aSortingMode;

  // actually do sorting
  nsNavHistoryContainerResultNode::SortComparator comparator =
      nsNavHistoryContainerResultNode::GetSortingComparator(aSortingMode);
  if (comparator) {
    nsNavHistory* history = nsNavHistory::GetHistoryService();
    NS_ENSURE_TRUE(history, NS_ERROR_OUT_OF_MEMORY);
    mRootNode->RecursiveSort(mSortingAnnotation.get(), comparator);
  }

  if (mView) {
    mView->SortingChanged(aSortingMode);
    mView->InvalidateAll();
  }
  return NS_OK;
}

NS_IMETHODIMP
nsNavHistoryResult::GetSortingAnnotation(nsACString& _result) {
  _result.Assign(mSortingAnnotation);
  return NS_OK;
}

NS_IMETHODIMP
nsNavHistoryResult::SetSortingAnnotation(const nsACString& aSortingAnnotation) {
  mSortingAnnotation.Assign(aSortingAnnotation);
  return NS_OK;
}

// nsNavHistoryResult::Viewer (nsINavHistoryResult)

NS_IMETHODIMP
nsNavHistoryResult::GetViewer(nsINavHistoryResultViewer** aViewer)
{
  *aViewer = mView;
  NS_IF_ADDREF(*aViewer);
  return NS_OK;
}
NS_IMETHODIMP
nsNavHistoryResult::SetViewer(nsINavHistoryResultViewer* aViewer)
{
  mView = aViewer;
  if (aViewer)
    aViewer->SetResult(this);
  return NS_OK;
}


// nsNavHistoryResult::GetRoot (nsINavHistoryResult)
//
NS_IMETHODIMP
nsNavHistoryResult::GetRoot(nsINavHistoryContainerResultNode** aRoot)
{
  if (! mRootNode) {
    NS_NOTREACHED("Root is null");
    *aRoot = nsnull;
    return NS_ERROR_FAILURE;
  }
  return mRootNode->QueryInterface(NS_GET_IID(nsINavHistoryContainerResultNode),
                                   reinterpret_cast<void**>(aRoot));
}


// nsINavBookmarkObserver implementation

// Here, it is important that we create a COPY of the observer array. Some
// observers will requery themselves, which may cause the observer array to
// be modified or added to.
#define ENUMERATE_BOOKMARK_FOLDER_OBSERVERS(_folderId, _functionCall) \
  { \
    FolderObserverList* _fol = BookmarkFolderObserversForId(_folderId, PR_FALSE); \
    if (_fol) { \
      FolderObserverList _listCopy(*_fol); \
      for (PRUint32 _fol_i = 0; _fol_i < _listCopy.Length(); _fol_i ++) { \
        if (_listCopy[_fol_i]) \
          _listCopy[_fol_i]->_functionCall; \
      } \
    } \
  }
#define ENUMERATE_ALL_BOOKMARKS_OBSERVERS(_functionCall) \
  { \
    nsTArray<nsNavHistoryQueryResultNode*> observerCopy(mAllBookmarksObservers); \
    for (PRUint32 _obs_i = 0; _obs_i < observerCopy.Length(); _obs_i ++) { \
      if (observerCopy[_obs_i]) \
      observerCopy[_obs_i]->_functionCall; \
    } \
  }
#define ENUMERATE_HISTORY_OBSERVERS(_functionCall) \
  { \
    nsTArray<nsNavHistoryQueryResultNode*> observerCopy(mHistoryObservers); \
    for (PRUint32 _obs_i = 0; _obs_i < observerCopy.Length(); _obs_i ++) { \
      if (observerCopy[_obs_i]) \
      observerCopy[_obs_i]->_functionCall; \
    } \
  }

// nsNavHistoryResult::OnBeginUpdateBatch (nsINavBookmark/HistoryObserver)

NS_IMETHODIMP
nsNavHistoryResult::OnBeginUpdateBatch()
{
  mBatchInProgress = PR_TRUE;
  ENUMERATE_HISTORY_OBSERVERS(OnBeginUpdateBatch());
  ENUMERATE_ALL_BOOKMARKS_OBSERVERS(OnBeginUpdateBatch());
  return NS_OK;
}


// nsNavHistoryResult::OnEndUpdateBatch (nsINavBookmark/HistoryObserver)

NS_IMETHODIMP
nsNavHistoryResult::OnEndUpdateBatch()
{
  NS_ASSERTION(mBatchInProgress, "EndUpdateBatch without a begin");
  mBatchInProgress = PR_FALSE;
  ENUMERATE_HISTORY_OBSERVERS(OnEndUpdateBatch());
  ENUMERATE_ALL_BOOKMARKS_OBSERVERS(OnEndUpdateBatch());
  return NS_OK;
}


// nsNavHistoryResult::OnItemAdded (nsINavBookmarkObserver)

NS_IMETHODIMP
nsNavHistoryResult::OnItemAdded(PRInt64 aItemId,
                                PRInt64 aFolder,
                                PRInt32 aIndex)
{
  ENUMERATE_BOOKMARK_FOLDER_OBSERVERS(aFolder,
      OnItemAdded(aItemId, aFolder, aIndex));
  ENUMERATE_HISTORY_OBSERVERS(OnItemAdded(aItemId, aFolder, aIndex));
  ENUMERATE_ALL_BOOKMARKS_OBSERVERS(OnItemAdded(aItemId, aFolder, aIndex));
  return NS_OK;
}


// nsNavHistoryResult::OnItemRemoved (nsINavBookmarkObserver)

NS_IMETHODIMP
nsNavHistoryResult::OnItemRemoved(PRInt64 aItemId,
                                  PRInt64 aFolder, PRInt32 aIndex)
{
  ENUMERATE_BOOKMARK_FOLDER_OBSERVERS(aFolder,
      OnItemRemoved(aItemId, aFolder, aIndex));
  ENUMERATE_ALL_BOOKMARKS_OBSERVERS(
      OnItemRemoved(aItemId, aFolder, aIndex));
  ENUMERATE_HISTORY_OBSERVERS(
      OnItemRemoved(aItemId, aFolder, aIndex));
  return NS_OK;
}


// nsNavHistoryResult::OnItemChanged (nsINavBookmarkObserver)

NS_IMETHODIMP
nsNavHistoryResult::OnItemChanged(PRInt64 aItemId,
                                  const nsACString &aProperty,
                                  PRBool aIsAnnotationProperty,
                                  const nsACString &aValue)
{
  nsNavBookmarks* bookmarkService = nsNavBookmarks::GetBookmarksService();
  NS_ENSURE_TRUE(bookmarkService, NS_ERROR_OUT_OF_MEMORY);

  ENUMERATE_ALL_BOOKMARKS_OBSERVERS(
    OnItemChanged(aItemId, aProperty, aIsAnnotationProperty, aValue));

  PRUint16 itemType;
  nsresult rv = bookmarkService->GetItemType(aItemId, &itemType);
  NS_ENSURE_SUCCESS(rv, rv);

  // Note: folder-nodes set their own bookmark observer only once they're
  // opened, meaning we cannot optimize this code path for changes done to
  // folder-nodes.

  // Find the changed items under the folders list
  PRInt64 folderId;
  rv = bookmarkService->GetFolderIdForItem(aItemId, &folderId);
  NS_ENSURE_SUCCESS(rv, rv);

  FolderObserverList* list = BookmarkFolderObserversForId(folderId, PR_FALSE);
  if (!list)
    return NS_OK;

  for (PRUint32 i = 0; i < list->Length(); i++) {
    nsNavHistoryFolderResultNode* folder = list->ElementAt(i);
    if (folder) {
      PRUint32 nodeIndex;
      nsNavHistoryResultNode* node = folder->FindChildById(aItemId, &nodeIndex);
      // if ExcludeItems is true we don't update non visible items
      if (node &&
          (!folder->mOptions->ExcludeItems() ||
           !(node->IsURI() || node->IsSeparator())) &&
          folder->StartIncrementalUpdate()) {
        node->OnItemChanged(aItemId, aProperty, aIsAnnotationProperty, aValue);
      }
    }
  }

  // Note: we do NOT call history observers in this case. This notification is
  // the same as other history notification, except that here we know the item
  // is a bookmark. History observers will handle the history notification
  // instead.
  return NS_OK;
}

// nsNavHistoryResult::OnItemVisited (nsINavBookmarkObserver)

NS_IMETHODIMP
nsNavHistoryResult::OnItemVisited(PRInt64 aItemId, PRInt64 aVisitId,
                                  PRTime aVisitTime)
{
  nsresult rv;
  nsNavBookmarks* bookmarkService = nsNavBookmarks::GetBookmarksService();
  NS_ENSURE_TRUE(bookmarkService, NS_ERROR_OUT_OF_MEMORY);

  // find the folder to notify about this item
  PRInt64 folderId;
  rv = bookmarkService->GetFolderIdForItem(aItemId, &folderId);
  NS_ENSURE_SUCCESS(rv, rv);
  ENUMERATE_BOOKMARK_FOLDER_OBSERVERS(folderId,
      OnItemVisited(aItemId, aVisitId, aVisitTime));
  ENUMERATE_ALL_BOOKMARKS_OBSERVERS(
      OnItemVisited(aItemId, aVisitId, aVisitTime));
  // Note: we do NOT call history observers in this case. This notification is
  // the same as OnVisit, except that here we know the item is a bookmark.
  // History observers will handle the history notification instead.
  return NS_OK;
}


// nsNavHistoryResult::OnItemMoved (nsINavBookmarkObserver)
//
//    Need to notify both the source and the destination folders (if they
//    are different).

NS_IMETHODIMP
nsNavHistoryResult::OnItemMoved(PRInt64 aItemId,
                                PRInt64 aOldParent, PRInt32 aOldIndex,
                                PRInt64 aNewParent, PRInt32 aNewIndex)
{
  { // scope for loop index for VC6's broken for loop scoping
    ENUMERATE_BOOKMARK_FOLDER_OBSERVERS(aOldParent,
        OnItemMoved(aItemId, aOldParent, aOldIndex, aNewParent, aNewIndex));
  }
  if (aNewParent != aOldParent) {
    ENUMERATE_BOOKMARK_FOLDER_OBSERVERS(aNewParent,
        OnItemMoved(aItemId, aOldParent, aOldIndex, aNewParent, aNewIndex));
  }
  ENUMERATE_ALL_BOOKMARKS_OBSERVERS(OnItemMoved(aItemId, aOldParent, aOldIndex,
                                                aNewParent, aNewIndex));
  ENUMERATE_HISTORY_OBSERVERS(OnItemMoved(aItemId, aOldParent, aOldIndex,
                                          aNewParent, aNewIndex));
  return NS_OK;
}

// nsNavHistoryResult::OnVisit (nsINavHistoryObserver)

NS_IMETHODIMP
nsNavHistoryResult::OnVisit(nsIURI* aURI, PRInt64 aVisitId, PRTime aTime,
                            PRInt64 aSessionId, PRInt64 aReferringId,
                            PRUint32 aTransitionType, PRUint32* aAdded)
{
  PRUint32 added = 0;

  ENUMERATE_HISTORY_OBSERVERS(OnVisit(aURI, aVisitId, aTime, aSessionId,
                                      aReferringId, aTransitionType, &added));

  if (!added && mRootNode->mExpanded) {
    nsresult rv;

    // None of registered query observers has accepted our URI, this means,
    // that a matching query either was not expanded or it does not exist.
    // If it just was not expanded, we can ignore it, but if it did not
    // exist, we have to add the query to the right place.
    PRUint32 resultType = mRootNode->mOptions->ResultType();
    nsNavHistoryResultNode * siteRoot = mRootNode;
    nsCAutoString dateRange;

    // For day based queries we just check whether the first item is Today,
    if (resultType == nsINavHistoryQueryOptions::RESULTS_AS_DATE_QUERY ||
        resultType == nsINavHistoryQueryOptions::RESULTS_AS_DATE_SITE_QUERY) {
      nsNavHistory* history = nsNavHistory::GetHistoryService();
      NS_ENSURE_TRUE(history, 0);

      // code borrowed from xpfe/components/history/src/nsGlobalHistory.cpp
      // pass in a pre-normalized now and a date, and we'll find
      // the difference since midnight on each of the days.
      //
      // USECS_PER_DAY == PR_USEC_PER_SEC * 60 * 60 * 24;
      static const PRInt64 USECS_PER_DAY = LL_INIT(20, 500654080);

      dateRange = nsPrintfCString(255,
        "&beginTime=%lld&endTime=%lld",
        history->NormalizeTime(
          nsINavHistoryQuery::TIME_RELATIVE_TODAY, 0),
        history->NormalizeTime(
          nsINavHistoryQuery::TIME_RELATIVE_TODAY, USECS_PER_DAY));

      PRBool todayIsMissing = PR_FALSE;
      PRUint32 childCount;
      rv = mRootNode->GetChildCount(&childCount);
      NS_ENSURE_SUCCESS(rv, rv);

      nsCAutoString todayLabel;
      history->GetStringFromName(
        NS_LITERAL_STRING("finduri-AgeInDays-is-0").get(), todayLabel);

      if (!childCount) {
        todayIsMissing = PR_TRUE;
      } else {
        nsCOMPtr<nsINavHistoryResultNode> firstChild;
        rv = mRootNode->GetChild(0, getter_AddRefs(firstChild));
        NS_ENSURE_SUCCESS(rv, rv);

        nsCAutoString title;
        rv = firstChild->GetTitle( title);
        NS_ENSURE_SUCCESS(rv, rv);

        if (todayLabel.Equals(title)) {
          siteRoot = static_cast<nsNavHistoryResultNode *>(firstChild.get());
        } else {
          todayIsMissing = PR_TRUE;
        }
      }

      if (todayIsMissing) { // Add "Today"
        nsCAutoString queryUri;
        queryUri = nsPrintfCString(255,
          "place:type=%ld&sort=%ld%s",
          resultType == nsINavHistoryQueryOptions::RESULTS_AS_DATE_QUERY
            ?nsINavHistoryQueryOptions::RESULTS_AS_URI
            :nsINavHistoryQueryOptions::RESULTS_AS_SITE_QUERY,
          nsINavHistoryQueryOptions::SORT_BY_TITLE_ASCENDING,
          dateRange.get());

        nsRefPtr<nsNavHistoryQueryResultNode> todayNode;
        todayNode = new nsNavHistoryQueryResultNode(todayLabel, 
                                                    EmptyCString(), queryUri);
        rv = mRootNode->InsertChildAt( todayNode, 0);
        NS_ENSURE_SUCCESS(rv, rv);
      }

      // "Today" was missing or we had day query
      if (resultType == nsINavHistoryQueryOptions::RESULTS_AS_DATE_QUERY || 
          todayIsMissing)
        return NS_OK; // No more processing necessary
    }

    if (siteRoot->IsQuery() && siteRoot->GetAsQuery()->mContentsValid &&
         (resultType == nsINavHistoryQueryOptions::RESULTS_AS_DATE_SITE_QUERY ||
          resultType == nsINavHistoryQueryOptions::RESULTS_AS_SITE_QUERY)) {
      nsCAutoString host;
      rv = aURI->GetAsciiHost(host);
      NS_ENSURE_SUCCESS(rv, rv);

      nsCAutoString queryUri;
      queryUri = nsPrintfCString(255,
        "place:type=%ld&sort=%ld&domain=%s&domainIsHost=true%s",
        nsINavHistoryQueryOptions::RESULTS_AS_URI,
        nsINavHistoryQueryOptions::SORT_BY_TITLE_ASCENDING,
        host.get(),
        dateRange.get());

      nsRefPtr<nsNavHistoryQueryResultNode> siteNode;
      siteNode = new nsNavHistoryQueryResultNode(host, EmptyCString(), queryUri);
      rv = siteRoot->GetAsContainer()->InsertSortedChild(
               siteNode, PR_FALSE, PR_TRUE/*Ignore duplicates*/);
      NS_ENSURE_SUCCESS(rv, rv);
    }
  }

  return NS_OK;
}


// nsNavHistoryResult::OnTitleChanged (nsINavHistoryObserver)

NS_IMETHODIMP
nsNavHistoryResult::OnTitleChanged(nsIURI* aURI, const nsAString& aPageTitle)
{
  ENUMERATE_HISTORY_OBSERVERS(OnTitleChanged(aURI, aPageTitle));
  return NS_OK;
}


// nsNavHistoryResult::OnDeleteURI (nsINavHistoryObserver)

NS_IMETHODIMP
nsNavHistoryResult::OnDeleteURI(nsIURI *aURI)
{
  ENUMERATE_HISTORY_OBSERVERS(OnDeleteURI(aURI));
  return NS_OK;
}


// nsNavHistoryResult::OnClearHistory (nsINavHistoryObserver)

NS_IMETHODIMP
nsNavHistoryResult::OnClearHistory()
{
  ENUMERATE_HISTORY_OBSERVERS(OnClearHistory());
  return NS_OK;
}


// nsNavHistoryResult::OnPageChanged (nsINavHistoryObserver)

NS_IMETHODIMP
nsNavHistoryResult::OnPageChanged(nsIURI *aURI,
                                  PRUint32 aWhat, const nsAString &aValue)
{
  ENUMERATE_HISTORY_OBSERVERS(OnPageChanged(aURI, aWhat, aValue));
  return NS_OK;
}


// nsNavHistoryResult;:OnPageExpired (nsINavHistoryObserver)
//
//    Don't do anything when pages expire. Perhaps we want to find the item
//    to delete it.

NS_IMETHODIMP
nsNavHistoryResult::OnPageExpired(nsIURI* aURI, PRTime aVisitTime,
                                  PRBool aWholeEntry)
{
  return NS_OK;
}
