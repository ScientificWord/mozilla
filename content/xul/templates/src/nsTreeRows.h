/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 *   Chris Waterson <waterson@netscape.com>
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

#ifndef nsTreeRows_h__
#define nsTreeRows_h__

#include "nsCOMPtr.h"
#include "nsTArray.h"
#include "pldhash.h"
#include "nsIXULTemplateResult.h"
#include "nsTemplateMatch.h"
#include "nsIRDFResource.h"


/**
 * This class maintains the state of the XUL tree builder's
 * rows. It maps a row number to the nsTemplateMatch object that
 * populates the row.
 */
class nsTreeRows
{
public:
    class iterator;
    friend class iterator;

    enum Direction { eDirection_Forwards = +1, eDirection_Backwards = -1 };

    // N.B. these values are chosen to avoid problems with
    // sign-extension from the bit-packed Row structure.
    enum ContainerType {
        eContainerType_Unknown      =  0,
        eContainerType_Noncontainer =  1,
        eContainerType_Container    = -2
    };

    enum ContainerState {
        eContainerState_Unknown     =  0,
        eContainerState_Open        =  1,
        eContainerState_Closed      = -2
    };

    enum ContainerFill {
        eContainerFill_Unknown      =  0,
        eContainerFill_Empty        =  1,
        eContainerFill_Nonempty     = -2
    };

    class Subtree;

    /**
     * A row in the tree. Contains the match that the row
     * corresponds to, and a pointer to the row's subtree, if there
     * are any.
     */
    struct Row {
        nsTemplateMatch* mMatch;
        ContainerType    mContainerType  : 2;
        ContainerState   mContainerState : 2;
        ContainerFill    mContainerFill  : 2;
        
        Subtree*         mSubtree; // XXX eventually move to hashtable
    };

    /**
     * A subtree in the tree. A subtree contains rows, which may
     * contain other subtrees.
     */
    class Subtree {
    protected:
        friend class nsTreeRows; // so that it can access members, for now

        /**
         * The parent subtree; null if we're the root
         */
        Subtree* mParent;

        /**
         * The number of immediate children in this subtree
         */
        PRInt32 mCount;

        /**
         * The capacity of the subtree
         */
        PRInt32 mCapacity;

        /**
         * The total number of rows in this subtree, recursively
         * including child subtrees.
         */
        PRInt32 mSubtreeSize;

        /**
         * The array of rows in the subtree
         */
        Row* mRows;

    public:
        /**
         * Creates a subtree with the specified parent.
         */
        Subtree(Subtree* aParent)
            : mParent(aParent),
              mCount(0),
              mCapacity(0),
              mSubtreeSize(0),
              mRows(nsnull) {}

        ~Subtree();

        /**
         * Return the number of immediate child rows in the subtree
         */
        PRInt32 Count() const { return mCount; }

        /**
         * Return the number of rows in this subtree, as well as all
         * the subtrees it contains.
         */
        PRInt32 GetSubtreeSize() const { return mSubtreeSize; }

        /**
         * Retrieve the immediate child row at the specified index.
         */
        const Row& operator[](PRInt32 aIndex) const {
            NS_PRECONDITION(aIndex >= 0 && aIndex < mCount, "bad index");
            return mRows[aIndex]; }

        /**
         * Retrieve the immediate row at the specified index.
         */
        Row& operator[](PRInt32 aIndex) {
            NS_PRECONDITION(aIndex >= 0 && aIndex < mCount, "bad index");
            return mRows[aIndex]; }

        /**
         * Remove all rows from the subtree.
         */
        void Clear();

    protected:
        /**
         * Insert an immediate child row at the specified index.
         */
        iterator InsertRowAt(nsTemplateMatch* aMatch, PRInt32 aIndex);

        /**
         * Remove an immediate child row from the specified index.
         */
        void RemoveRowAt(PRInt32 aChildIndex);
    };

    friend class Subtree;

protected:
    /**
     * A link in the path through the view's tree.
     */
    struct Link {
        Subtree* mParent;
        PRInt32  mChildIndex;

        Link&
        operator=(const Link& aLink) {
            mParent = aLink.mParent;
            mChildIndex = aLink.mChildIndex;
            return *this; }

        PRBool
        operator==(const Link& aLink) const {
            return (mParent == aLink.mParent)
                && (mChildIndex == aLink.mChildIndex); }

        Subtree* GetParent() { return mParent; }
        const Subtree* GetParent() const { return mParent; }

        PRInt32 GetChildIndex() const { return mChildIndex; }

        Row& GetRow() { return (*mParent)[mChildIndex]; }
        const Row& GetRow() const { return (*mParent)[mChildIndex]; }
    };

public:
    /**
     * An iterator that can be used to traverse the tree view.
     */
    class iterator {
    protected:
        PRInt32 mRowIndex;
        nsAutoTArray<Link, 8> mLink;

        void Next();
        void Prev();

        friend class Subtree; // so InsertRowAt can initialize us
        friend class nsTreeRows; // so nsTreeRows can initialize us

        /**
         * Used by operator[]() to initialize an iterator.
         */
        void Append(Subtree* aParent, PRInt32 aChildIndex);

        /**
         * Used by InsertRowAt() to initialize an iterator.
         */
        void Push(Subtree *aParent, PRInt32 aChildIndex);

        /**
         * Used by operator[]() and InsertRowAt() to initialize an iterator.
         */
        void SetRowIndex(PRInt32 aRowIndex) { mRowIndex = aRowIndex; }

        /**
         * Handy accessors to the top element.
         */
        Link& GetTop() { return mLink[mLink.Length() - 1]; }
        const Link& GetTop() const { return mLink[mLink.Length() - 1]; }

    public:
        iterator() : mRowIndex(-1) {}

        iterator(const iterator& aIterator);
        iterator& operator=(const iterator& aIterator);

        PRBool operator==(const iterator& aIterator) const;

        PRBool operator!=(const iterator& aIterator) const {
            return !aIterator.operator==(*this); }

        const Row& operator*() const { return GetTop().GetRow(); }
        Row& operator*() { return GetTop().GetRow(); }

        const Row* operator->() const { return &(GetTop().GetRow()); }
        Row* operator->() { return &(GetTop().GetRow()); }

        iterator& operator++() { Next(); return *this; }
        iterator operator++(int) { iterator temp(*this); Next(); return temp; }
        iterator& operator--() { Prev(); return *this; }
        iterator operator--(int) { iterator temp(*this); Prev(); return temp; }

        /**
         * Return the current parent link
         */
        Subtree* GetParent() { return GetTop().GetParent(); }

        const Subtree* GetParent() const { return GetTop().GetParent(); }

        /**
         * Return the current child index
         */
        PRInt32 GetChildIndex() const { return GetTop().GetChildIndex(); }

        /**
         * Return the depth of the path the iterator is maintaining
         * into the tree.
         */
        PRInt32 GetDepth() const { return mLink.Length(); }

        /**
         * Return the current row index of the iterator
         */
        PRInt32 GetRowIndex() const { return mRowIndex; }

        /**
         * Pop the iterator up a level.
         */
        iterator& Pop() { mLink.SetLength(GetDepth() - 1); return *this; }
    };

    /**
     * Retrieve the first element in the view
     */
    iterator First();

    /**
     * Retrieve (one past) the last element in the view
     */
    iterator Last();

    /**
     * Find the row that contains the given resource
     */
    iterator FindByResource(nsIRDFResource* aResource);

    /**
     * Find the row that contains the result
     */
    iterator Find(nsIXULTemplateResult* aResult);

    /**
     * Retrieve the ith element in the view
     */
    iterator operator[](PRInt32 aIndex);

    nsTreeRows() : mRoot(nsnull) {}
    ~nsTreeRows() {}

    /**
     * Ensure that a child subtree exists within the specified parent
     * at the specified child index within the parent. (In other
     * words, create a subtree if one doesn't already exist.)
     */
    Subtree*
    EnsureSubtreeFor(Subtree* aParent, PRInt32 aChildIndex);

    /**
     * Ensure that a child subtree exists at the iterator's position.
     */
    Subtree*
    EnsureSubtreeFor(iterator& aIterator) {
        return EnsureSubtreeFor(aIterator.GetParent(),
                                aIterator.GetChildIndex()); }

    /**
     * Get the child subtree for the specified parent at the specified
     * child index. Optionally return the child subtree's size. Will
     * return `null' if no subtree exists.
     */
    Subtree*
    GetSubtreeFor(const Subtree* aParent,
                  PRInt32 aChildIndex,
                  PRInt32* aSubtreeSize = nsnull);

    /**
     * Retrieve the size of the subtree within the specified parent.
     */
    PRInt32
    GetSubtreeSizeFor(const Subtree* aParent,
                      PRInt32 aChildIndex) {
        PRInt32 size;
        GetSubtreeFor(aParent, aChildIndex, &size);
        return size; }

    /**
     * Retrieve the size of the subtree within the specified parent.
     */
    PRInt32
    GetSubtreeSizeFor(const iterator& aIterator) {
        PRInt32 size;
        GetSubtreeFor(aIterator.GetParent(), aIterator.GetChildIndex(), &size);
        return size; }

    /**
     * Remove the specified subtree for a row, leaving the row itself
     * intact.
     */
    void
    RemoveSubtreeFor(Subtree* aParent, PRInt32 aChildIndex);

    /**
     * Remove the specified subtree for a row, leaving the row itself
     * intact.
     */
    void
    RemoveSubtreeFor(iterator& aIterator) {
        RemoveSubtreeFor(aIterator.GetParent(), aIterator.GetChildIndex()); }

    /**
     * Remove the specified row from the view
     */
    PRInt32
    RemoveRowAt(iterator& aIterator) {
        iterator temp = aIterator--;
        Subtree* parent = temp.GetParent();
        parent->RemoveRowAt(temp.GetChildIndex());
        InvalidateCachedRow();
        return parent->Count(); }

    /**
     * Insert a new match into the view
     */
    iterator
    InsertRowAt(nsTemplateMatch* aMatch, Subtree* aSubtree, PRInt32 aChildIndex) {
        InvalidateCachedRow();
        return aSubtree->InsertRowAt(aMatch, aChildIndex); }

    /**
     * Raw access to the rows; e.g., for sorting.
     */
    Row*
    GetRowsFor(Subtree* aSubtree) { return aSubtree->mRows; }

    /**
     * Remove all of the rows
     */
    void Clear();

    /**
     * Return the total number of rows in the tree view.
     */
    PRInt32 Count() const { return mRoot.GetSubtreeSize(); }

    /**
     * Retrieve the root subtree
     */
    Subtree* GetRoot() { return &mRoot; }

    /**
     * Set the root resource for the view
     */
    void SetRootResource(nsIRDFResource* aResource) {
        mRootResource = aResource; }

    /**
     * Retrieve the root resource for the view
     */
    nsIRDFResource* GetRootResource() {
        return mRootResource.get(); }

    /**
     * Invalidate the cached row; e.g., because the view has changed
     * in a way that would corrupt the iterator.
     */
    void
    InvalidateCachedRow() { mLastRow = iterator(); }

protected:
    /**
     * The root subtree.
     */
    Subtree mRoot;

    /**
     * The root resource for the view
     */
    nsCOMPtr<nsIRDFResource> mRootResource;

    /**
     * The last row that was asked for by operator[]. By remembering
     * this, we can usually avoid the O(n) search through the row
     * array to find the row at the specified index.
     */
    iterator mLastRow;
};


#endif // nsTreeRows_h__
