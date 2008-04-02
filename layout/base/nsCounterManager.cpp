/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
// vim:cindent:ai:sw=4:ts=4:et:
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
 * The Original Code is nsCounterManager.
 *
 * The Initial Developer of the Original Code is the Mozilla Foundation.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   L. David Baron <dbaron@dbaron.org> (original author)
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

/* implementation of CSS counters (for numbering things) */

#include "nsCounterManager.h"
#include "nsBulletFrame.h" // legacy location for list style type to text code
#include "nsContentUtils.h"

// assign the correct |mValueAfter| value to a node that has been inserted
// Should be called immediately after calling |Insert|.
void nsCounterUseNode::Calc(nsCounterList *aList)
{
    NS_ASSERTION(!aList->IsDirty(),
                 "Why are we calculating with a dirty list?");
    mValueAfter = aList->ValueBefore(this);
}

// assign the correct |mValueAfter| value to a node that has been inserted
// Should be called immediately after calling |Insert|.
void nsCounterChangeNode::Calc(nsCounterList *aList)
{
    NS_ASSERTION(!aList->IsDirty(),
                 "Why are we calculating with a dirty list?");
    if (mType == RESET) {
        mValueAfter = mChangeValue;
    } else {
        NS_ASSERTION(mType == INCREMENT, "invalid type");
        mValueAfter = aList->ValueBefore(this) + mChangeValue;
    }
}

// The text that should be displayed for this counter.
void
nsCounterUseNode::GetText(nsString& aResult)
{
    aResult.Truncate();

    nsAutoVoidArray stack;
    stack.AppendElement(static_cast<nsCounterNode*>(this));

    if (mAllCounters && mScopeStart)
        for (nsCounterNode *n = mScopeStart; n->mScopePrev; n = n->mScopeStart)
            stack.AppendElement(n->mScopePrev);

    PRInt32 style = mCounterStyle->Item(mAllCounters ? 2 : 1).GetIntValue();
    const PRUnichar* separator;
    if (mAllCounters)
        separator = mCounterStyle->Item(1).GetStringBufferValue();

    for (PRInt32 i = stack.Count() - 1;; --i) {
        nsCounterNode *n = static_cast<nsCounterNode*>(stack[i]);
        nsBulletFrame::AppendCounterText(style, n->mValueAfter, aResult);
        if (i == 0)
            break;
        NS_ASSERTION(mAllCounters, "yikes, separator is uninitialized");
        aResult.Append(separator);
    }
}

void
nsCounterList::SetScope(nsCounterNode *aNode)
{
    // This function is responsible for setting |mScopeStart| and
    // |mScopePrev| (whose purpose is described in nsCounterManager.h).
    // We do this by starting from the node immediately preceding
    // |aNode| in content tree order, which is reasonably likely to be
    // the previous element in our scope (or, for a reset, the previous
    // element in the containing scope, which is what we want).  If
    // we're not in the same scope that it is, then it's too deep in the
    // frame tree, so we walk up parent scopes until we find something
    // appropriate.

    if (aNode == First()) {
        aNode->mScopeStart = nsnull;
        aNode->mScopePrev = nsnull;
        return;
    }

    // Get the content node for aNode's rendering object's *parent*,
    // since scope includes siblings, so we want a descendant check on
    // parents.  If aNode is for a pseudo-element, then the parent
    // rendering object is the frame's content; if aNode is for an
    // element, then the parent rendering object is the frame's
    // content's parent.
    nsIContent *nodeContent = aNode->mPseudoFrame->GetContent();
    if (!aNode->mPseudoFrame->GetStyleContext()->GetPseudoType()) {
        nodeContent = nodeContent->GetParent();
    }

    for (nsCounterNode *prev = Prev(aNode), *start;
         prev; prev = start->mScopePrev) {
        // If |prev| starts a scope (because it's a real or implied
        // reset), we want it as the scope start rather than the start
        // of its enclosing scope.  Otherwise, there's no enclosing
        // scope, so the next thing in prev's scope shares its scope
        // start.
        start = (prev->mType == nsCounterNode::RESET || !prev->mScopeStart)
                  ? prev : prev->mScopeStart;

        // |startContent| is analogous to |nodeContent| (see above).
        nsIContent *startContent = start->mPseudoFrame->GetContent();
        if (!start->mPseudoFrame->GetStyleContext()->GetPseudoType()) {
            startContent = startContent->GetParent();
        }
        NS_ASSERTION(nodeContent || !startContent,
                     "null check on startContent should be sufficient to "
                     "null check nodeContent as well, since if nodeContent "
                     "is for the root, startContent (which is before it) "
                     "must be too");

             // A reset's outer scope can't be a scope created by a sibling.
        if (!(aNode->mType == nsCounterNode::RESET &&
              nodeContent == startContent) &&
              // everything is inside the root (except the case above,
              // a second reset on the root)
            (!startContent ||
             nsContentUtils::ContentIsDescendantOf(nodeContent,
                                                   startContent))) {
            aNode->mScopeStart = start;
            aNode->mScopePrev  = prev;
            return;
        }
    }

    aNode->mScopeStart = nsnull;
    aNode->mScopePrev  = nsnull;
}

void
nsCounterList::RecalcAll()
{
    mDirty = PR_FALSE;

    nsCounterNode *node = First();
    if (!node)
        return;

    do {
        SetScope(node);
        node->Calc(this);

        if (node->mType == nsCounterNode::USE) {
            nsCounterUseNode *useNode = node->UseNode();
            // Null-check mText, since if the frame constructor isn't
            // batching, we could end up here while the node is being
            // constructed.
            if (useNode->mText) {
                nsAutoString text;
                useNode->GetText(text);
                useNode->mText->SetData(text);
            }
        }
    } while ((node = Next(node)) != First());
}

nsCounterManager::nsCounterManager()
{
    mNames.Init(16);
}

PRBool
nsCounterManager::AddCounterResetsAndIncrements(nsIFrame *aFrame)
{
    const nsStyleContent *styleContent = aFrame->GetStyleContent();
    if (!styleContent->CounterIncrementCount() &&
        !styleContent->CounterResetCount())
        return PR_FALSE;

    // Add in order, resets first, so all the comparisons will be optimized
    // for addition at the end of the list.
    PRInt32 i, i_end;
    PRBool dirty = PR_FALSE;
    for (i = 0, i_end = styleContent->CounterResetCount(); i != i_end; ++i)
        dirty |= AddResetOrIncrement(aFrame, i,
                                     styleContent->GetCounterResetAt(i),
                                     nsCounterChangeNode::RESET);
    for (i = 0, i_end = styleContent->CounterIncrementCount(); i != i_end; ++i)
        dirty |= AddResetOrIncrement(aFrame, i,
                                     styleContent->GetCounterIncrementAt(i),
                                     nsCounterChangeNode::INCREMENT);
    return dirty;
}

PRBool
nsCounterManager::AddResetOrIncrement(nsIFrame *aFrame, PRInt32 aIndex,
                                      const nsStyleCounterData *aCounterData,
                                      nsCounterNode::Type aType)
{
    nsCounterChangeNode *node =
        new nsCounterChangeNode(aFrame, aType, aCounterData->mValue, aIndex);
    if (!node)
        return PR_FALSE;

    nsCounterList *counterList = CounterListFor(aCounterData->mCounter);
    if (!counterList) {
        NS_NOTREACHED("CounterListFor failed (should only happen on OOM)");
        return PR_FALSE;
    }

    counterList->Insert(node);
    if (!counterList->IsLast(node)) {
        // Tell the caller it's responsible for recalculating the entire
        // list.
        counterList->SetDirty();
        return PR_TRUE;
    }

    // Don't call Calc() if the list is already dirty -- it'll be recalculated
    // anyway, and trying to calculate with a dirty list doesn't work.
    if (NS_LIKELY(!counterList->IsDirty())) {
        node->Calc(counterList);
    }
    return PR_FALSE;
}

nsCounterList*
nsCounterManager::CounterListFor(const nsSubstring& aCounterName)
{
    // XXX Why doesn't nsTHashtable provide an API that allows us to use
    // get/put in one hashtable lookup?
    nsCounterList *counterList;
    if (!mNames.Get(aCounterName, &counterList)) {
        counterList = new nsCounterList();
        if (!counterList)
            return nsnull;
        if (!mNames.Put(aCounterName, counterList)) {
            delete counterList;
            return nsnull;
        }
    }
    return counterList;
}

PR_STATIC_CALLBACK(PLDHashOperator)
RecalcDirtyLists(const nsAString& aKey, nsCounterList* aList, void* aClosure)
{
    if (aList->IsDirty())
        aList->RecalcAll();
    return PL_DHASH_NEXT;
}

void
nsCounterManager::RecalcAll()
{
    mNames.EnumerateRead(RecalcDirtyLists, nsnull);
}

struct DestroyNodesData {
    DestroyNodesData(nsIFrame *aFrame)
        : mFrame(aFrame)
        , mDestroyedAny(PR_FALSE)
    {
    }

    nsIFrame *mFrame;
    PRBool mDestroyedAny;
};

PR_STATIC_CALLBACK(PLDHashOperator)
DestroyNodesInList(const nsAString& aKey, nsCounterList* aList, void* aClosure)
{
    DestroyNodesData *data = static_cast<DestroyNodesData*>(aClosure);
    if (aList->DestroyNodesFor(data->mFrame)) {
        data->mDestroyedAny = PR_TRUE;
        aList->SetDirty();
    }
    return PL_DHASH_NEXT;
}

PRBool
nsCounterManager::DestroyNodesFor(nsIFrame *aFrame)
{
    DestroyNodesData data(aFrame);
    mNames.EnumerateRead(DestroyNodesInList, &data);
    return data.mDestroyedAny;
}

#ifdef DEBUG
PR_STATIC_CALLBACK(PLDHashOperator)
DumpList(const nsAString& aKey, nsCounterList* aList, void* aClosure)
{
    printf("Counter named \"%s\":\n", NS_ConvertUTF16toUTF8(aKey).get());
    nsCounterNode *node = aList->First();

    if (node) {
        PRInt32 i = 0;
        do {
            const char *types[] = { "RESET", "INCREMENT", "USE" };
            printf("  Node #%d @%p frame=%p index=%d type=%s valAfter=%d\n"
                   "       scope-start=%p scope-prev=%p",
                   i++, (void*)node, (void*)node->mPseudoFrame,
                   node->mContentIndex, types[node->mType], node->mValueAfter,
                   (void*)node->mScopeStart, (void*)node->mScopePrev);
            if (node->mType == nsCounterNode::USE) {
                nsAutoString text;
                node->UseNode()->GetText(text);
                printf(" text=%s", NS_ConvertUTF16toUTF8(text).get());
            }
            printf("\n");
        } while ((node = aList->Next(node)) != aList->First());
    }
    return PL_DHASH_NEXT;
}

void
nsCounterManager::Dump()
{
    printf("\n\nCounter Manager Lists:\n");
    mNames.EnumerateRead(DumpList, nsnull);
    printf("\n\n");
}
#endif
