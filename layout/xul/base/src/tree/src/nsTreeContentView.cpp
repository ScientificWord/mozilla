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
 * The Initial Developer of the Original Code is Jan Varga.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Brian Ryner <bryner@brianryner.com>
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

#include "nsINameSpaceManager.h"
#include "nsGkAtoms.h"
#include "nsIBoxObject.h"
#include "nsTreeUtils.h"
#include "nsTreeContentView.h"
#include "nsChildIterator.h"
#include "nsIDOMHTMLOptionElement.h"
#include "nsIDOMHTMLOptGroupElement.h"
#include "nsIDOMClassInfo.h"
#include "nsIEventStateManager.h"
#include "nsINodeInfo.h"
#include "nsIXULSortService.h"

// A content model view implementation for the tree.

#define ROW_FLAG_CONTAINER      0x01
#define ROW_FLAG_OPEN           0x02
#define ROW_FLAG_EMPTY          0x04
#define ROW_FLAG_SEPARATOR      0x08

class Row
{
  public:
    static Row*
    Create(nsFixedSizeAllocator& aAllocator,
           nsIContent* aContent, PRInt32 aParentIndex) {
      void* place = aAllocator.Alloc(sizeof(Row));
      return place ? ::new(place) Row(aContent, aParentIndex) : nsnull;
    }

    static void
    Destroy(nsFixedSizeAllocator& aAllocator, Row* aRow) {
      aRow->~Row();
      aAllocator.Free(aRow, sizeof(*aRow));
    }

    Row(nsIContent* aContent, PRInt32 aParentIndex)
      : mContent(aContent), mParentIndex(aParentIndex),
        mSubtreeSize(0), mFlags(0) {
    }

    ~Row() {
    }

    void SetContainer(PRBool aContainer) {
      aContainer ? mFlags |= ROW_FLAG_CONTAINER : mFlags &= ~ROW_FLAG_CONTAINER;
    }
    PRBool IsContainer() { return mFlags & ROW_FLAG_CONTAINER; }

    void SetOpen(PRBool aOpen) {
      aOpen ? mFlags |= ROW_FLAG_OPEN : mFlags &= ~ROW_FLAG_OPEN;
    }
    PRBool IsOpen() { return !!(mFlags & ROW_FLAG_OPEN); }

    void SetEmpty(PRBool aEmpty) {
      aEmpty ? mFlags |= ROW_FLAG_EMPTY : mFlags &= ~ROW_FLAG_EMPTY;
    }
    PRBool IsEmpty() { return !!(mFlags & ROW_FLAG_EMPTY); }

    void SetSeparator(PRBool aSeparator) {
      aSeparator ? mFlags |= ROW_FLAG_SEPARATOR : mFlags &= ~ROW_FLAG_SEPARATOR;
    }
    PRBool IsSeparator() { return !!(mFlags & ROW_FLAG_SEPARATOR); }

    // Weak reference to a content item.
    nsIContent*         mContent;

    // The parent index of the item, set to -1 for the top level items.
    PRInt32             mParentIndex;

    // Subtree size for this item.
    PRInt32             mSubtreeSize;

  private:
    // Hide so that only Create() and Destroy() can be used to
    // allocate and deallocate from the heap
    static void* operator new(size_t) CPP_THROW_NEW { return 0; } 
    static void operator delete(void*, size_t) {}

    // State flags
    PRInt8		mFlags;
};


// We don't reference count the reference to the document
// If the document goes away first, we'll be informed and we
// can drop our reference.
// If we go away first, we'll get rid of ourselves from the
// document's observer list.

nsTreeContentView::nsTreeContentView(void) :
  mBoxObject(nsnull),
  mSelection(nsnull),
  mRoot(nsnull),
  mDocument(nsnull),
  mUpdateSelection(PR_FALSE)
{
  static const size_t kBucketSizes[] = {
    sizeof(Row)
  };
  static const PRInt32 kNumBuckets = sizeof(kBucketSizes) / sizeof(size_t);
  static const PRInt32 kInitialSize = 16;

  mAllocator.Init("nsTreeContentView", kBucketSizes, kNumBuckets, kInitialSize);
}

nsTreeContentView::~nsTreeContentView(void)
{
  // Remove ourselves from mDocument's observers.
  if (mDocument)
    mDocument->RemoveObserver(this);
}

nsresult
NS_NewTreeContentView(nsITreeView** aResult)
{
  *aResult = new nsTreeContentView;
  if (! *aResult)
    return NS_ERROR_OUT_OF_MEMORY;
  NS_ADDREF(*aResult);
  return NS_OK;
}

NS_IMPL_ADDREF(nsTreeContentView)
NS_IMPL_RELEASE(nsTreeContentView)

NS_INTERFACE_MAP_BEGIN(nsTreeContentView)
  NS_INTERFACE_MAP_ENTRY(nsITreeView)
  NS_INTERFACE_MAP_ENTRY(nsITreeContentView)
  NS_INTERFACE_MAP_ENTRY(nsIDocumentObserver)
  NS_INTERFACE_MAP_ENTRY(nsIMutationObserver)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsITreeContentView)
  NS_INTERFACE_MAP_ENTRY_DOM_CLASSINFO(TreeContentView)
NS_INTERFACE_MAP_END

NS_IMETHODIMP
nsTreeContentView::GetRowCount(PRInt32* aRowCount)
{
  *aRowCount = mRows.Count();

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::GetSelection(nsITreeSelection** aSelection)
{
  NS_IF_ADDREF(*aSelection = mSelection);

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::SetSelection(nsITreeSelection* aSelection)
{
  mSelection = aSelection;
  if (!mSelection || !mUpdateSelection)
    return NS_OK;

  mUpdateSelection = PR_FALSE;

  mSelection->SetSelectEventsSuppressed(PR_TRUE);
  for (PRInt32 i = 0; i < mRows.Count(); ++i) {
    Row* row = (Row*)mRows[i];
    nsCOMPtr<nsIDOMHTMLOptionElement> optEl = do_QueryInterface(row->mContent);
    if (optEl) {
      PRBool isSelected;
      optEl->GetSelected(&isSelected);
      if (isSelected)
        mSelection->ToggleSelect(i);
    }
  }
  mSelection->SetSelectEventsSuppressed(PR_FALSE);

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::GetRowProperties(PRInt32 aIndex, nsISupportsArray* aProperties)
{
  NS_ENSURE_ARG_POINTER(aProperties);
  NS_PRECONDITION(aIndex >= 0 && aIndex < mRows.Count(), "bad index");
  if (aIndex < 0 || aIndex >= mRows.Count())
    return NS_ERROR_INVALID_ARG;   

  Row* row = (Row*)mRows[aIndex];
  nsCOMPtr<nsIContent> realRow;
  if (row->IsSeparator())
    realRow = row->mContent;
  else
    nsTreeUtils::GetImmediateChild(row->mContent, nsGkAtoms::treerow, getter_AddRefs(realRow));

  if (realRow) {
    nsAutoString properties;
    realRow->GetAttr(kNameSpaceID_None, nsGkAtoms::properties, properties);
    if (!properties.IsEmpty())
      nsTreeUtils::TokenizeProperties(properties, aProperties);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::GetCellProperties(PRInt32 aRow, nsITreeColumn* aCol, nsISupportsArray* aProperties)
{
  NS_ENSURE_ARG_POINTER(aCol);
  NS_ENSURE_ARG_POINTER(aProperties);
  NS_PRECONDITION(aRow >= 0 && aRow < mRows.Count(), "bad row");
  if (aRow < 0 || aRow >= mRows.Count())
    return NS_ERROR_INVALID_ARG;   

  Row* row = (Row*)mRows[aRow];
  nsCOMPtr<nsIContent> realRow;
  nsTreeUtils::GetImmediateChild(row->mContent, nsGkAtoms::treerow, getter_AddRefs(realRow));
  if (realRow) {
    nsIContent* cell = GetCell(realRow, aCol);
    if (cell) {
      nsAutoString properties;
      cell->GetAttr(kNameSpaceID_None, nsGkAtoms::properties, properties);
      if (!properties.IsEmpty())
        nsTreeUtils::TokenizeProperties(properties, aProperties);
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::GetColumnProperties(nsITreeColumn* aCol, nsISupportsArray* aProperties)
{
  NS_ENSURE_ARG_POINTER(aCol);
  NS_ENSURE_ARG_POINTER(aProperties);
  nsCOMPtr<nsIDOMElement> element;
  aCol->GetElement(getter_AddRefs(element));

  nsAutoString properties;
  element->GetAttribute(NS_LITERAL_STRING("properties"), properties);

  if (!properties.IsEmpty())
    nsTreeUtils::TokenizeProperties(properties, aProperties);

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::IsContainer(PRInt32 aIndex, PRBool* _retval)
{
  NS_PRECONDITION(aIndex >= 0 && aIndex < mRows.Count(), "bad index");
  if (aIndex < 0 || aIndex >= mRows.Count())
    return NS_ERROR_INVALID_ARG;   

  *_retval = ((Row*)mRows[aIndex])->IsContainer();

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::IsContainerOpen(PRInt32 aIndex, PRBool* _retval)
{
  NS_PRECONDITION(aIndex >= 0 && aIndex < mRows.Count(), "bad index");
  if (aIndex < 0 || aIndex >= mRows.Count())
    return NS_ERROR_INVALID_ARG;   

  *_retval = ((Row*)mRows[aIndex])->IsOpen();

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::IsContainerEmpty(PRInt32 aIndex, PRBool* _retval)
{
  NS_PRECONDITION(aIndex >= 0 && aIndex < mRows.Count(), "bad index");
  if (aIndex < 0 || aIndex >= mRows.Count())
    return NS_ERROR_INVALID_ARG;   

  *_retval = ((Row*)mRows[aIndex])->IsEmpty();

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::IsSeparator(PRInt32 aIndex, PRBool *_retval)
{
  NS_PRECONDITION(aIndex >= 0 && aIndex < mRows.Count(), "bad index");
  if (aIndex < 0 || aIndex >= mRows.Count())
    return NS_ERROR_INVALID_ARG;   

  *_retval = ((Row*)mRows[aIndex])->IsSeparator();

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::IsSorted(PRBool *_retval)
{
  *_retval = PR_FALSE;

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::CanDrop(PRInt32 aIndex, PRInt32 aOrientation, PRBool *_retval)
{
  NS_PRECONDITION(aIndex >= 0 && aIndex < mRows.Count(), "bad index");
  if (aIndex < 0 || aIndex >= mRows.Count())
    return NS_ERROR_INVALID_ARG;   

  *_retval = PR_FALSE;
 
  return NS_OK;
}
 
NS_IMETHODIMP
nsTreeContentView::Drop(PRInt32 aRow, PRInt32 aOrientation)
{
  NS_PRECONDITION(aRow >= 0 && aRow < mRows.Count(), "bad row");
  if (aRow < 0 || aRow >= mRows.Count())
    return NS_ERROR_INVALID_ARG;   

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::GetParentIndex(PRInt32 aRowIndex, PRInt32* _retval)
{
  NS_PRECONDITION(aRowIndex >= 0 && aRowIndex < mRows.Count(), "bad row index");
  if (aRowIndex < 0 || aRowIndex >= mRows.Count())
    return NS_ERROR_INVALID_ARG;   

  *_retval = ((Row*)mRows[aRowIndex])->mParentIndex;

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::HasNextSibling(PRInt32 aRowIndex, PRInt32 aAfterIndex, PRBool* _retval)
{
  NS_PRECONDITION(aRowIndex >= 0 && aRowIndex < mRows.Count(), "bad row index");
  if (aRowIndex < 0 || aRowIndex >= mRows.Count())
    return NS_ERROR_INVALID_ARG;   

  // We have a next sibling if the row is not the last in the subtree.
  PRInt32 parentIndex = ((Row*)mRows[aRowIndex])->mParentIndex;
  if (parentIndex >= 0) {
    // Compute the last index in this subtree.
    PRInt32 lastIndex = parentIndex + ((Row*)mRows[parentIndex])->mSubtreeSize;
    Row* row = (Row*)mRows[lastIndex];
    while (row->mParentIndex != parentIndex) {
      lastIndex = row->mParentIndex;
      row = (Row*)mRows[lastIndex];
    }

    *_retval = aRowIndex < lastIndex;
  }
  else {
    *_retval = aRowIndex < mRows.Count() - 1;
  }

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::GetLevel(PRInt32 aIndex, PRInt32* _retval)
{
  NS_PRECONDITION(aIndex >= 0 && aIndex < mRows.Count(), "bad index");
  if (aIndex < 0 || aIndex >= mRows.Count())
    return NS_ERROR_INVALID_ARG;   

  PRInt32 level = 0;
  Row* row = (Row*)mRows[aIndex];
  while (row->mParentIndex >= 0) {
    level++;
    row = (Row*)mRows[row->mParentIndex];
  }
  *_retval = level;

  return NS_OK;
}

 NS_IMETHODIMP
nsTreeContentView::GetImageSrc(PRInt32 aRow, nsITreeColumn* aCol, nsAString& _retval)
{
  _retval.Truncate();
  NS_ENSURE_ARG_POINTER(aCol);
  NS_PRECONDITION(aRow >= 0 && aRow < mRows.Count(), "bad row");
  if (aRow < 0 || aRow >= mRows.Count())
    return NS_ERROR_INVALID_ARG;   

  Row* row = (Row*)mRows[aRow];

  nsCOMPtr<nsIContent> realRow;
  nsTreeUtils::GetImmediateChild(row->mContent, nsGkAtoms::treerow, getter_AddRefs(realRow));
  if (realRow) {
    nsIContent* cell = GetCell(realRow, aCol);
    if (cell)
      cell->GetAttr(kNameSpaceID_None, nsGkAtoms::src, _retval);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::GetProgressMode(PRInt32 aRow, nsITreeColumn* aCol, PRInt32* _retval)
{
  NS_ENSURE_ARG_POINTER(aCol);
  NS_PRECONDITION(aRow >= 0 && aRow < mRows.Count(), "bad row");
  if (aRow < 0 || aRow >= mRows.Count())
    return NS_ERROR_INVALID_ARG;   

  *_retval = nsITreeView::PROGRESS_NONE;

  Row* row = (Row*)mRows[aRow];

  nsCOMPtr<nsIContent> realRow;
  nsTreeUtils::GetImmediateChild(row->mContent, nsGkAtoms::treerow, getter_AddRefs(realRow));
  if (realRow) {
    nsIContent* cell = GetCell(realRow, aCol);
    if (cell) {
      static nsIContent::AttrValuesArray strings[] =
        {&nsGkAtoms::normal, &nsGkAtoms::undetermined, nsnull};
      switch (cell->FindAttrValueIn(kNameSpaceID_None, nsGkAtoms::mode,
                                    strings, eCaseMatters)) {
        case 0: *_retval = nsITreeView::PROGRESS_NORMAL; break;
        case 1: *_retval = nsITreeView::PROGRESS_UNDETERMINED; break;
      }
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::GetCellValue(PRInt32 aRow, nsITreeColumn* aCol, nsAString& _retval)
{
  _retval.Truncate();
  NS_ENSURE_ARG_POINTER(aCol);
  NS_PRECONDITION(aRow >= 0 && aRow < mRows.Count(), "bad row");
  if (aRow < 0 || aRow >= mRows.Count())
    return NS_ERROR_INVALID_ARG;   

  Row* row = (Row*)mRows[aRow];

  nsCOMPtr<nsIContent> realRow;
  nsTreeUtils::GetImmediateChild(row->mContent, nsGkAtoms::treerow, getter_AddRefs(realRow));
  if (realRow) {
    nsIContent* cell = GetCell(realRow, aCol);
    if (cell)
      cell->GetAttr(kNameSpaceID_None, nsGkAtoms::value, _retval);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::GetCellText(PRInt32 aRow, nsITreeColumn* aCol, nsAString& _retval)
{
  _retval.Truncate();
  NS_ENSURE_ARG_POINTER(aCol);
  NS_PRECONDITION(aRow >= 0 && aRow < mRows.Count(), "bad row");
  NS_PRECONDITION(aCol, "bad column");

  if (aRow < 0 || aRow >= mRows.Count() || !aCol)
    return NS_ERROR_INVALID_ARG;

  Row* row = (Row*)mRows[aRow];

  // Check for a "label" attribute - this is valid on an <treeitem>
  // or an <option>, with a single implied column.
  if (row->mContent->GetAttr(kNameSpaceID_None, nsGkAtoms::label, _retval)
      && !_retval.IsEmpty())
    return NS_OK;

  nsIAtom *rowTag = row->mContent->Tag();
  if (rowTag == nsGkAtoms::option &&
      row->mContent->IsNodeOfType(nsINode::eHTML)) {
    // Use the text node child as the label
    nsCOMPtr<nsIDOMHTMLOptionElement> elem = do_QueryInterface(row->mContent);
    elem->GetText(_retval);
  }
  else if (rowTag == nsGkAtoms::optgroup &&
           row->mContent->IsNodeOfType(nsINode::eHTML)) {
    nsCOMPtr<nsIDOMHTMLOptGroupElement> elem = do_QueryInterface(row->mContent);
    elem->GetLabel(_retval);
  }
  else if (rowTag == nsGkAtoms::treeitem &&
           row->mContent->IsNodeOfType(nsINode::eXUL)) {
    nsCOMPtr<nsIContent> realRow;
    nsTreeUtils::GetImmediateChild(row->mContent, nsGkAtoms::treerow,
                                   getter_AddRefs(realRow));
    if (realRow) {
      nsIContent* cell = GetCell(realRow, aCol);
      if (cell)
        cell->GetAttr(kNameSpaceID_None, nsGkAtoms::label, _retval);
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::SetTree(nsITreeBoxObject* aTree)
{
  ClearRows();

  mBoxObject = aTree;

  if (aTree && !mRoot) {
    // Get our root element
    nsCOMPtr<nsIBoxObject> boxObject = do_QueryInterface(mBoxObject);
    nsCOMPtr<nsIDOMElement> element;
    boxObject->GetElement(getter_AddRefs(element));

    mRoot = do_QueryInterface(element);

    // Add ourselves to document's observers.
    nsIDocument* document = mRoot->GetDocument();
    if (document) {
      document->AddObserver(this);
      mDocument = document;
    }

    nsCOMPtr<nsIDOMElement> bodyElement;
    mBoxObject->GetTreeBody(getter_AddRefs(bodyElement));
    if (bodyElement) {
      mBody = do_QueryInterface(bodyElement);
      PRInt32 index = 0;
      Serialize(mBody, -1, &index, mRows);
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::ToggleOpenState(PRInt32 aIndex)
{
  NS_PRECONDITION(aIndex >= 0 && aIndex < mRows.Count(), "bad index");
  if (aIndex < 0 || aIndex >= mRows.Count())
    return NS_ERROR_INVALID_ARG;   

  // We don't serialize content right here, since content might be generated
  // lazily.
  Row* row = (Row*)mRows[aIndex];

  if (row->mContent->Tag() == nsGkAtoms::optgroup &&
      row->mContent->IsNodeOfType(nsINode::eHTML)) {
    // we don't use an attribute for optgroup's open state
    if (row->IsOpen())
      CloseContainer(aIndex);
    else
      OpenContainer(aIndex);
  }
  else {
    if (row->IsOpen())
      row->mContent->SetAttr(kNameSpaceID_None, nsGkAtoms::open, NS_LITERAL_STRING("false"), PR_TRUE);
    else
      row->mContent->SetAttr(kNameSpaceID_None, nsGkAtoms::open, NS_LITERAL_STRING("true"), PR_TRUE);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::CycleHeader(nsITreeColumn* aCol)
{
  NS_ENSURE_ARG_POINTER(aCol);

  if (!mRoot)
    return NS_OK;

  nsCOMPtr<nsIDOMElement> element;
  aCol->GetElement(getter_AddRefs(element));
  if (element) {
    nsCOMPtr<nsIContent> column = do_QueryInterface(element);
    nsAutoString sort;
    column->GetAttr(kNameSpaceID_None, nsGkAtoms::sort, sort);
    if (!sort.IsEmpty()) {
      nsCOMPtr<nsIXULSortService> xs = do_GetService("@mozilla.org/xul/xul-sort-service;1");
      if (xs) {
        nsAutoString sortdirection;
        static nsIContent::AttrValuesArray strings[] =
          {&nsGkAtoms::ascending, &nsGkAtoms::descending, nsnull};
        switch (column->FindAttrValueIn(kNameSpaceID_None,
                                        nsGkAtoms::sortDirection,
                                        strings, eCaseMatters)) {
          case 0: sortdirection.AssignLiteral("descending"); break;
          case 1: sortdirection.AssignLiteral("natural"); break;
          default: sortdirection.AssignLiteral("ascending"); break;
        }

        nsCOMPtr<nsIDOMNode> rootnode = do_QueryInterface(mRoot);
        xs->Sort(rootnode, sort, sortdirection);
      }
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::SelectionChanged()
{
  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::CycleCell(PRInt32 aRow, nsITreeColumn* aCol)
{
  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::IsEditable(PRInt32 aRow, nsITreeColumn* aCol, PRBool* _retval)
{
  *_retval = PR_FALSE;
  NS_ENSURE_ARG_POINTER(aCol);
  NS_PRECONDITION(aRow >= 0 && aRow < mRows.Count(), "bad row");
  if (aRow < 0 || aRow >= mRows.Count())
    return NS_ERROR_INVALID_ARG;   

  *_retval = PR_TRUE;

  Row* row = (Row*)mRows[aRow];

  nsCOMPtr<nsIContent> realRow;
  nsTreeUtils::GetImmediateChild(row->mContent, nsGkAtoms::treerow, getter_AddRefs(realRow));
  if (realRow) {
    nsIContent* cell = GetCell(realRow, aCol);
    if (cell && cell->AttrValueIs(kNameSpaceID_None, nsGkAtoms::editable,
                                  nsGkAtoms::_false, eCaseMatters)) {
      *_retval = PR_FALSE;
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::IsSelectable(PRInt32 aRow, nsITreeColumn* aCol, PRBool* _retval)
{
  NS_PRECONDITION(aRow >= 0 && aRow < mRows.Count(), "bad row");
  if (aRow < 0 || aRow >= mRows.Count())
    return NS_ERROR_INVALID_ARG;   

  *_retval = PR_TRUE;

  Row* row = (Row*)mRows[aRow];

  nsCOMPtr<nsIContent> realRow;
  nsTreeUtils::GetImmediateChild(row->mContent, nsGkAtoms::treerow, getter_AddRefs(realRow));
  if (realRow) {
    nsIContent* cell = GetCell(realRow, aCol);
    if (cell && cell->AttrValueIs(kNameSpaceID_None, nsGkAtoms::selectable,
                                  nsGkAtoms::_false, eCaseMatters)) {
      *_retval = PR_FALSE;
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::SetCellValue(PRInt32 aRow, nsITreeColumn* aCol, const nsAString& aValue)
{
  NS_ENSURE_ARG_POINTER(aCol);
  NS_PRECONDITION(aRow >= 0 && aRow < mRows.Count(), "bad row");
  if (aRow < 0 || aRow >= mRows.Count())
    return NS_ERROR_INVALID_ARG;   

  Row* row = (Row*)mRows[aRow];

  nsCOMPtr<nsIContent> realRow;
  nsTreeUtils::GetImmediateChild(row->mContent, nsGkAtoms::treerow, getter_AddRefs(realRow));
  if (realRow) {
    nsIContent* cell = GetCell(realRow, aCol);
    if (cell)
      cell->SetAttr(kNameSpaceID_None, nsGkAtoms::value, aValue, PR_TRUE);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::SetCellText(PRInt32 aRow, nsITreeColumn* aCol, const nsAString& aValue)
{
  NS_ENSURE_ARG_POINTER(aCol);
  NS_PRECONDITION(aRow >= 0 && aRow < mRows.Count(), "bad row");
  if (aRow < 0 || aRow >= mRows.Count())
    return NS_ERROR_INVALID_ARG;   

  Row* row = (Row*)mRows[aRow];

  nsCOMPtr<nsIContent> realRow;
  nsTreeUtils::GetImmediateChild(row->mContent, nsGkAtoms::treerow, getter_AddRefs(realRow));
  if (realRow) {
    nsIContent* cell = GetCell(realRow, aCol);
    if (cell)
      cell->SetAttr(kNameSpaceID_None, nsGkAtoms::label, aValue, PR_TRUE);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::PerformAction(const PRUnichar* aAction)
{
  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::PerformActionOnRow(const PRUnichar* aAction, PRInt32 aRow)
{
  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::PerformActionOnCell(const PRUnichar* aAction, PRInt32 aRow, nsITreeColumn* aCol)
{
  return NS_OK;
}


NS_IMETHODIMP
nsTreeContentView::GetItemAtIndex(PRInt32 aIndex, nsIDOMElement** _retval)
{
  NS_PRECONDITION(aIndex >= 0 && aIndex < mRows.Count(), "bad index");
  if (aIndex < 0 || aIndex >= mRows.Count())
    return NS_ERROR_INVALID_ARG;   

  Row* row = (Row*)mRows[aIndex];
  row->mContent->QueryInterface(NS_GET_IID(nsIDOMElement), (void**)_retval);

  return NS_OK;
}

NS_IMETHODIMP
nsTreeContentView::GetIndexOfItem(nsIDOMElement* aItem, PRInt32* _retval)
{
  nsCOMPtr<nsIContent> content = do_QueryInterface(aItem);
  *_retval = FindContent(content);

  return NS_OK;
}

void
nsTreeContentView::ContentStatesChanged(nsIDocument* aDocument,
                                        nsIContent* aContent1,
                                        nsIContent* aContent2,
                                        PRInt32 aStateMask)
{
  if (!aContent1 || !mSelection ||
      !aContent1->IsNodeOfType(nsINode::eHTML) ||
      !(aStateMask & NS_EVENT_STATE_CHECKED))
    return;

  if (aContent1->Tag() == nsGkAtoms::option) {
    // update the selected state for this node
    PRInt32 index = FindContent(aContent1);
    if (index >= 0)
      mSelection->ToggleSelect(index);
  }
}

void
nsTreeContentView::AttributeChanged(nsIDocument *aDocument,
                                    nsIContent*  aContent,
                                    PRInt32      aNameSpaceID,
                                    nsIAtom*     aAttribute,
                                    PRInt32      aModType,
                                    PRUint32     aStateMask)
{
  // Make sure this notification concerns us.
  // First check the tag to see if it's one that we care about.
  nsIAtom *tag = aContent->Tag();

  if (mBoxObject && (aContent == mRoot || aContent == mBody)) {
    mBoxObject->ClearStyleAndImageCaches();
    mBoxObject->Invalidate();
  }

  if (aContent->IsNodeOfType(nsINode::eXUL)) {
    if (tag != nsGkAtoms::treecol &&
        tag != nsGkAtoms::treeitem &&
        tag != nsGkAtoms::treeseparator &&
        tag != nsGkAtoms::treerow &&
        tag != nsGkAtoms::treecell)
      return;
  }
  else {
    return;
  }

  // If we have a legal tag, go up to the tree/select and make sure
  // that it's ours.

  for (nsIContent* element = aContent; element != mBody; element = element->GetParent()) {
    if (!element)
      return; // this is not for us
    nsIAtom *parentTag = element->Tag();
    if ((element->IsNodeOfType(nsINode::eXUL) && parentTag == nsGkAtoms::tree) ||
        (element->IsNodeOfType(nsINode::eHTML) && parentTag == nsGkAtoms::select))
      return; // this is not for us
  }

  // Handle changes of the hidden attribute.
  if (aAttribute == nsGkAtoms::hidden &&
     (tag == nsGkAtoms::treeitem || tag == nsGkAtoms::treeseparator)) {
    PRBool hidden = aContent->AttrValueIs(kNameSpaceID_None,
                                          nsGkAtoms::hidden,
                                          nsGkAtoms::_true, eCaseMatters);
 
    PRInt32 index = FindContent(aContent);
    if (hidden && index >= 0) {
      // Hide this row along with its children.
      PRInt32 count = RemoveRow(index);
      if (mBoxObject)
        mBoxObject->RowCountChanged(index, -count);
    }
    else if (!hidden && index < 0) {
      // Show this row along with its children.
      nsCOMPtr<nsIContent> parent = aContent->GetParent();
      if (parent) {
        InsertRowFor(parent, aContent);
      }
    }

    return;
  }

  if (tag == nsGkAtoms::treecol) {
    if (aAttribute == nsGkAtoms::properties) {
      if (mBoxObject) {
        nsCOMPtr<nsITreeColumns> cols;
        mBoxObject->GetColumns(getter_AddRefs(cols));
        if (cols) {
          nsCOMPtr<nsIDOMElement> element = do_QueryInterface(aContent);
          nsCOMPtr<nsITreeColumn> col;
          cols->GetColumnFor(element, getter_AddRefs(col));
          mBoxObject->InvalidateColumn(col);
        }
      }
    }
  }
  else if (tag == nsGkAtoms::treeitem) {
    PRInt32 index = FindContent(aContent);
    if (index >= 0) {
      Row* row = (Row*)mRows[index];
      if (aAttribute == nsGkAtoms::container) {
        PRBool isContainer =
          aContent->AttrValueIs(kNameSpaceID_None, nsGkAtoms::container,
                                nsGkAtoms::_true, eCaseMatters);
        row->SetContainer(isContainer);
        if (mBoxObject)
          mBoxObject->InvalidateRow(index);
      }
      else if (aAttribute == nsGkAtoms::open) {
        PRBool isOpen =
          aContent->AttrValueIs(kNameSpaceID_None, nsGkAtoms::open,
                                nsGkAtoms::_true, eCaseMatters);
        PRBool wasOpen = row->IsOpen();
        if (! isOpen && wasOpen)
          CloseContainer(index);
        else if (isOpen && ! wasOpen)
          OpenContainer(index);
      }
      else if (aAttribute == nsGkAtoms::empty) {
        PRBool isEmpty =
          aContent->AttrValueIs(kNameSpaceID_None, nsGkAtoms::empty,
                                nsGkAtoms::_true, eCaseMatters);
        row->SetEmpty(isEmpty);
        if (mBoxObject)
          mBoxObject->InvalidateRow(index);
      }
    }
  }
  else if (tag == nsGkAtoms::treeseparator) {
    PRInt32 index = FindContent(aContent);
    if (index >= 0) {
      if (aAttribute == nsGkAtoms::properties && mBoxObject) {
        mBoxObject->InvalidateRow(index);
      }
    }
  }
  else if (tag == nsGkAtoms::treerow) {
    if (aAttribute == nsGkAtoms::properties) {
      nsCOMPtr<nsIContent> parent = aContent->GetParent();
      if (parent) {
        PRInt32 index = FindContent(parent);
        if (index >= 0 && mBoxObject) {
          mBoxObject->InvalidateRow(index);
        }
      }
    }
  }
  else if (tag == nsGkAtoms::treecell) {
    if (aAttribute == nsGkAtoms::ref ||
        aAttribute == nsGkAtoms::properties ||
        aAttribute == nsGkAtoms::mode ||
        aAttribute == nsGkAtoms::src ||
        aAttribute == nsGkAtoms::value ||
        aAttribute == nsGkAtoms::label) {
      nsIContent* parent = aContent->GetParent();
      if (parent) {
        nsCOMPtr<nsIContent> grandParent = parent->GetParent();
        if (grandParent) {
          PRInt32 index = FindContent(grandParent);
          if (index >= 0 && mBoxObject) {
            // XXX Should we make an effort to invalidate only cell ?
            mBoxObject->InvalidateRow(index);
          }
        }
      }
    }
  }
}

void
nsTreeContentView::ContentAppended(nsIDocument *aDocument,
                                   nsIContent* aContainer,
                                   PRInt32     aNewIndexInContainer)
{
  PRUint32 childCount = aContainer->GetChildCount();
  while ((PRUint32)aNewIndexInContainer < childCount) {
    nsIContent *child = aContainer->GetChildAt(aNewIndexInContainer);
    ContentInserted(aDocument, aContainer, child, aNewIndexInContainer);
    aNewIndexInContainer++;
  }
}

void
nsTreeContentView::ContentInserted(nsIDocument *aDocument,
                                   nsIContent* aContainer,
                                   nsIContent* aChild,
                                   PRInt32 aIndexInContainer)
{
  NS_ASSERTION(aChild, "null ptr");

  // Make sure this notification concerns us.
  // First check the tag to see if it's one that we care about.
  nsIAtom *childTag = aChild->Tag();

  if (aChild->IsNodeOfType(nsINode::eHTML)) {
    if (childTag != nsGkAtoms::option &&
        childTag != nsGkAtoms::optgroup)
      return;
  }
  else if (aChild->IsNodeOfType(nsINode::eXUL)) {
    if (childTag != nsGkAtoms::treeitem &&
        childTag != nsGkAtoms::treeseparator &&
        childTag != nsGkAtoms::treechildren &&
        childTag != nsGkAtoms::treerow &&
        childTag != nsGkAtoms::treecell)
      return;
  }
  else {
    return;
  }

  // If we have a legal tag, go up to the tree/select and make sure
  // that it's ours.

  for (nsIContent* element = aContainer; element != mBody; element = element->GetParent()) {
    if (!element)
      return; // this is not for us
    nsIAtom *parentTag = element->Tag();
    if ((element->IsNodeOfType(nsINode::eXUL) && parentTag == nsGkAtoms::tree) ||
        (element->IsNodeOfType(nsINode::eHTML) && parentTag == nsGkAtoms::select))
      return; // this is not for us
  }

  if (childTag == nsGkAtoms::treechildren) {
    PRInt32 index = FindContent(aContainer);
    if (index >= 0) {
      Row* row = (Row*)mRows[index];
      row->SetEmpty(PR_FALSE);
      if (mBoxObject)
        mBoxObject->InvalidateRow(index);
      if (row->IsContainer() && row->IsOpen()) {
        PRInt32 count = EnsureSubtree(index);
        if (mBoxObject)
          mBoxObject->RowCountChanged(index + 1, count);
      }
    }
  }
  else if (childTag == nsGkAtoms::treeitem ||
           childTag == nsGkAtoms::treeseparator) {
    InsertRowFor(aContainer, aChild);
  }
  else if (childTag == nsGkAtoms::treerow) {
    PRInt32 index = FindContent(aContainer);
    if (index >= 0 && mBoxObject)
      mBoxObject->InvalidateRow(index);
  }
  else if (childTag == nsGkAtoms::treecell) {
    nsCOMPtr<nsIContent> parent = aContainer->GetParent();
    if (parent) {
      PRInt32 index = FindContent(parent);
      if (index >= 0 && mBoxObject)
        mBoxObject->InvalidateRow(index);
    }
  }
  else if (childTag == nsGkAtoms::optgroup) {
    InsertRowFor(aContainer, aChild);
  }
  else if (childTag == nsGkAtoms::option) {
    PRInt32 parentIndex = FindContent(aContainer);
    PRInt32 count = InsertRow(parentIndex, aIndexInContainer, aChild);
    if (mBoxObject)
      mBoxObject->RowCountChanged(parentIndex + aIndexInContainer + 1, count);
  }
}

void
nsTreeContentView::ContentRemoved(nsIDocument *aDocument,
                                     nsIContent* aContainer,
                                     nsIContent* aChild,
                                     PRInt32 aIndexInContainer)
{
  NS_ASSERTION(aChild, "null ptr");

  // Make sure this notification concerns us.
  // First check the tag to see if it's one that we care about.
  nsIAtom *tag = aChild->Tag();

  if (aChild->IsNodeOfType(nsINode::eHTML)) {
    if (tag != nsGkAtoms::option &&
        tag != nsGkAtoms::optgroup)
      return;
  }
  else if (aChild->IsNodeOfType(nsINode::eXUL)) {
    if (tag != nsGkAtoms::treeitem &&
        tag != nsGkAtoms::treeseparator &&
        tag != nsGkAtoms::treechildren &&
        tag != nsGkAtoms::treerow &&
        tag != nsGkAtoms::treecell)
      return;
  }
  else {
    return;
  }

  // If we have a legal tag, go up to the tree/select and make sure
  // that it's ours.

  for (nsIContent* element = aContainer; element != mBody; element = element->GetParent()) {
    if (!element)
      return; // this is not for us
    nsIAtom *parentTag = element->Tag();
    if ((element->IsNodeOfType(nsINode::eXUL) && parentTag == nsGkAtoms::tree) ||
        (element->IsNodeOfType(nsINode::eHTML) && parentTag == nsGkAtoms::select))
      return; // this is not for us
  }

  if (tag == nsGkAtoms::treechildren) {
    PRInt32 index = FindContent(aContainer);
    if (index >= 0) {
      Row* row = (Row*)mRows[index];
      row->SetEmpty(PR_TRUE);
      PRInt32 count = RemoveSubtree(index);
      // Invalidate also the row to update twisty.
      if (mBoxObject) {
        mBoxObject->InvalidateRow(index);
        mBoxObject->RowCountChanged(index + 1, -count);
      }
    }
  }
  else if (tag == nsGkAtoms::treeitem ||
           tag == nsGkAtoms::treeseparator ||
           tag == nsGkAtoms::option ||
           tag == nsGkAtoms::optgroup
          ) {
    PRInt32 index = FindContent(aChild);
    if (index >= 0) {
      PRInt32 count = RemoveRow(index);
      if (mBoxObject)
        mBoxObject->RowCountChanged(index, -count);
    }
  }
  else if (tag == nsGkAtoms::treerow) {
    PRInt32 index = FindContent(aContainer);
    if (index >= 0 && mBoxObject)
      mBoxObject->InvalidateRow(index);
  }
  else if (tag == nsGkAtoms::treecell) {
    nsCOMPtr<nsIContent> parent = aContainer->GetParent();
    if (parent) {
      PRInt32 index = FindContent(parent);
      if (index >= 0 && mBoxObject)
        mBoxObject->InvalidateRow(index);
    }
  }
}

void
nsTreeContentView::NodeWillBeDestroyed(const nsINode* aNode)
{
  ClearRows();
}


// Recursively serialize content, starting with aContent.
void
nsTreeContentView::Serialize(nsIContent* aContent, PRInt32 aParentIndex, PRInt32* aIndex, nsVoidArray& aRows)
{
  ChildIterator iter, last;
  for (ChildIterator::Init(aContent, &iter, &last); iter != last; ++iter) {
    nsCOMPtr<nsIContent> content = *iter;
    nsIAtom *tag = content->Tag();
    PRInt32 count = aRows.Count();

    if (content->IsNodeOfType(nsINode::eXUL)) {
      if (tag == nsGkAtoms::treeitem)
        SerializeItem(content, aParentIndex, aIndex, aRows);
      else if (tag == nsGkAtoms::treeseparator)
        SerializeSeparator(content, aParentIndex, aIndex, aRows);
    }
    else if (content->IsNodeOfType(nsINode::eHTML)) {
      if (tag == nsGkAtoms::option)
        SerializeOption(content, aParentIndex, aIndex, aRows);
      else if (tag == nsGkAtoms::optgroup)
        SerializeOptGroup(content, aParentIndex, aIndex, aRows);
    }
    *aIndex += aRows.Count() - count;
  }
}

void
nsTreeContentView::SerializeItem(nsIContent* aContent, PRInt32 aParentIndex, PRInt32* aIndex, nsVoidArray& aRows)
{
  if (aContent->AttrValueIs(kNameSpaceID_None, nsGkAtoms::hidden,
                            nsGkAtoms::_true, eCaseMatters))
    return;

  Row* row = Row::Create(mAllocator, aContent, aParentIndex);
  aRows.AppendElement(row);

  if (aContent->AttrValueIs(kNameSpaceID_None, nsGkAtoms::container,
                            nsGkAtoms::_true, eCaseMatters)) {
    row->SetContainer(PR_TRUE);
    if (aContent->AttrValueIs(kNameSpaceID_None, nsGkAtoms::open,
                              nsGkAtoms::_true, eCaseMatters)) {
      row->SetOpen(PR_TRUE);
      nsCOMPtr<nsIContent> child;
      nsTreeUtils::GetImmediateChild(aContent, nsGkAtoms::treechildren, getter_AddRefs(child));
      if (child) {
        // Now, recursively serialize our child.
        PRInt32 count = aRows.Count();
        PRInt32 index = 0;
        Serialize(child, aParentIndex + *aIndex + 1, &index, aRows);
        row->mSubtreeSize += aRows.Count() - count;
      }
      else
        row->SetEmpty(PR_TRUE);
    } else if (aContent->AttrValueIs(kNameSpaceID_None, nsGkAtoms::empty,
                                     nsGkAtoms::_true, eCaseMatters)) {
      row->SetEmpty(PR_TRUE);
    }
  } 
}

void
nsTreeContentView::SerializeSeparator(nsIContent* aContent, PRInt32 aParentIndex, PRInt32* aIndex, nsVoidArray& aRows)
{
  if (aContent->AttrValueIs(kNameSpaceID_None, nsGkAtoms::hidden,
                            nsGkAtoms::_true, eCaseMatters))
    return;

  Row* row = Row::Create(mAllocator, aContent, aParentIndex);
  row->SetSeparator(PR_TRUE);
  aRows.AppendElement(row);
}

void
nsTreeContentView::SerializeOption(nsIContent* aContent, PRInt32 aParentIndex,
                                   PRInt32* aIndex, nsVoidArray& aRows)
{
  Row* row = Row::Create(mAllocator, aContent, aParentIndex);
  aRows.AppendElement(row);

  // This will happen before the TreeSelection is hooked up.  So, cache the selected
  // state in the row properties and update the selection when it is attached.

  nsCOMPtr<nsIDOMHTMLOptionElement> optEl = do_QueryInterface(aContent);
  PRBool isSelected;
  optEl->GetSelected(&isSelected);
  if (isSelected)
    mUpdateSelection = PR_TRUE;
}

void
nsTreeContentView::SerializeOptGroup(nsIContent* aContent, PRInt32 aParentIndex,
                                     PRInt32* aIndex, nsVoidArray& aRows)
{
  Row* row = Row::Create(mAllocator, aContent, aParentIndex);
  aRows.AppendElement(row);
  row->SetContainer(PR_TRUE);
  row->SetOpen(PR_TRUE);

  nsCOMPtr<nsIContent> child;
  nsTreeUtils::GetImmediateChild(aContent, nsGkAtoms::option, getter_AddRefs(child));
  if (child) {
    // Now, recursively serialize our child.
    PRInt32 count = aRows.Count();
    PRInt32 index = 0;
    Serialize(aContent, aParentIndex + *aIndex + 1, &index, aRows);
    row->mSubtreeSize += aRows.Count() - count;
  }
  else
    row->SetEmpty(PR_TRUE);
}

void
nsTreeContentView::GetIndexInSubtree(nsIContent* aContainer,
                                     nsIContent* aContent, PRInt32* aIndex)
{
  PRUint32 childCount = aContainer->GetChildCount();
  for (PRUint32 i = 0; i < childCount; i++) {
    nsIContent *content = aContainer->GetChildAt(i);

    if (content == aContent)
      break;

    nsIAtom *tag = content->Tag();

    if (content->IsNodeOfType(nsINode::eXUL)) {
      if (tag == nsGkAtoms::treeitem) {
        if (! content->AttrValueIs(kNameSpaceID_None, nsGkAtoms::hidden,
                                   nsGkAtoms::_true, eCaseMatters)) {
          (*aIndex)++;
          if (content->AttrValueIs(kNameSpaceID_None, nsGkAtoms::container,
                                   nsGkAtoms::_true, eCaseMatters) &&
              content->AttrValueIs(kNameSpaceID_None, nsGkAtoms::open,
                                   nsGkAtoms::_true, eCaseMatters)) {
            nsCOMPtr<nsIContent> child;
            nsTreeUtils::GetImmediateChild(content, nsGkAtoms::treechildren,
                                           getter_AddRefs(child));
            if (child)
              GetIndexInSubtree(child, aContent, aIndex);
          }
        }
      }
      else if (tag == nsGkAtoms::treeseparator) {
        if (! content->AttrValueIs(kNameSpaceID_None, nsGkAtoms::hidden,
                                   nsGkAtoms::_true, eCaseMatters))
          (*aIndex)++;
      }
    }
    else if (content->IsNodeOfType(nsINode::eHTML)) {
      if (tag == nsGkAtoms::optgroup) {
        (*aIndex)++;
        GetIndexInSubtree(content, aContent, aIndex);
      }
      else if (tag == nsGkAtoms::option)
        (*aIndex)++;
    }
  }
}

PRInt32
nsTreeContentView::EnsureSubtree(PRInt32 aIndex)
{
  Row* row = (Row*)mRows[aIndex];

  nsCOMPtr<nsIContent> child;
  if (row->mContent->Tag() == nsGkAtoms::optgroup)
    child = row->mContent;
  else {
    nsTreeUtils::GetImmediateChild(row->mContent, nsGkAtoms::treechildren, getter_AddRefs(child));
    if (! child) {
      return 0;
    }
  }

  nsAutoVoidArray rows;
  PRInt32 index = 0;
  Serialize(child, aIndex, &index, rows);
  mRows.InsertElementsAt(rows, aIndex + 1);
  PRInt32 count = rows.Count();

  row->mSubtreeSize += count;
  UpdateSubtreeSizes(row->mParentIndex, count);

  // Update parent indexes, but skip newly added rows.
  // They already have correct values.
  UpdateParentIndexes(aIndex, count + 1, count);

  return count;
}

PRInt32
nsTreeContentView::RemoveSubtree(PRInt32 aIndex)
{
  Row* row = (Row*)mRows[aIndex];
  PRInt32 count = row->mSubtreeSize;

  for(PRInt32 i = 0; i < count; i++) {
    Row* nextRow = (Row*)mRows[aIndex + i + 1];
    Row::Destroy(mAllocator, nextRow);
  }
  mRows.RemoveElementsAt(aIndex + 1, count);

  row->mSubtreeSize -= count;
  UpdateSubtreeSizes(row->mParentIndex, -count);

  UpdateParentIndexes(aIndex, 0, -count);

  return count;
}

void
nsTreeContentView::InsertRowFor(nsIContent* aParent, nsIContent* aChild)
{
  PRInt32 grandParentIndex = -1;
  PRBool insertRow = PR_FALSE;

  nsCOMPtr<nsIContent> grandParent = aParent->GetParent();
  nsIAtom* grandParentTag = grandParent->Tag();

  if ((grandParent->IsNodeOfType(nsINode::eXUL) && grandParentTag == nsGkAtoms::tree) ||
      (grandParent->IsNodeOfType(nsINode::eHTML) && grandParentTag == nsGkAtoms::select)
     ) {
    // Allow insertion to the outermost container.
    insertRow = PR_TRUE;
  }
  else {
    // Test insertion to an inner container.

    // First try to find this parent in our array of rows, if we find one
    // we can be sure that all other parents are open too.
    grandParentIndex = FindContent(grandParent);
    if (grandParentIndex >= 0) {
      // Got it, now test if it is open.
      if (((Row*)mRows[grandParentIndex])->IsOpen())
        insertRow = PR_TRUE;
    }
  }

  if (insertRow) {
    PRInt32 index = 0;
    GetIndexInSubtree(aParent, aChild, &index);

    PRInt32 count = InsertRow(grandParentIndex, index, aChild);
    if (mBoxObject)
      mBoxObject->RowCountChanged(grandParentIndex + index + 1, count);
  }
}

PRInt32
nsTreeContentView::InsertRow(PRInt32 aParentIndex, PRInt32 aIndex, nsIContent* aContent)
{
  nsAutoVoidArray rows;
  nsIAtom *tag = aContent->Tag();
  if (aContent->IsNodeOfType(nsINode::eXUL)) {
    if (tag == nsGkAtoms::treeitem)
      SerializeItem(aContent, aParentIndex, &aIndex, rows);
    else if (tag == nsGkAtoms::treeseparator)
      SerializeSeparator(aContent, aParentIndex, &aIndex, rows);
  }
  else if (aContent->IsNodeOfType(nsINode::eHTML)) {
    if (tag == nsGkAtoms::option)
      SerializeOption(aContent, aParentIndex, &aIndex, rows);
    else if (tag == nsGkAtoms::optgroup)
      SerializeOptGroup(aContent, aParentIndex, &aIndex, rows);
  }

  mRows.InsertElementsAt(rows, aParentIndex + aIndex + 1);
  PRInt32 count = rows.Count();

  UpdateSubtreeSizes(aParentIndex, count);

  // Update parent indexes, but skip added rows.
  // They already have correct values.
  UpdateParentIndexes(aParentIndex + aIndex, count + 1, count);

  return count;
}

PRInt32
nsTreeContentView::RemoveRow(PRInt32 aIndex)
{
  Row* row = (Row*)mRows[aIndex];
  PRInt32 count = row->mSubtreeSize + 1;
  PRInt32 parentIndex = row->mParentIndex;

  Row::Destroy(mAllocator, row);
  for(PRInt32 i = 1; i < count; i++) {
    Row* nextRow = (Row*)mRows[aIndex + i];
    Row::Destroy(mAllocator, nextRow);
  }
  mRows.RemoveElementsAt(aIndex, count);

  UpdateSubtreeSizes(parentIndex, -count);
  
  UpdateParentIndexes(aIndex, 0, -count);

  return count;
}

void
nsTreeContentView::ClearRows()
{
  for (PRInt32 i = 0; i < mRows.Count(); i++)
    Row::Destroy(mAllocator, (Row*)mRows[i]);
  mRows.Clear();
  mRoot = nsnull;
  mBody = nsnull;
  // Remove ourselves from mDocument's observers.
  if (mDocument) {
    mDocument->RemoveObserver(this);
    mDocument = nsnull;
  }
} 

void
nsTreeContentView::OpenContainer(PRInt32 aIndex)
{
  Row* row = (Row*)mRows[aIndex];
  row->SetOpen(PR_TRUE);

  PRInt32 count = EnsureSubtree(aIndex);
  if (mBoxObject) {
    mBoxObject->InvalidateRow(aIndex);
    mBoxObject->RowCountChanged(aIndex + 1, count);
  }
}

void
nsTreeContentView::CloseContainer(PRInt32 aIndex)
{
  Row* row = (Row*)mRows[aIndex];
  row->SetOpen(PR_FALSE);

  PRInt32 count = RemoveSubtree(aIndex);
  if (mBoxObject) {
    mBoxObject->InvalidateRow(aIndex);
    mBoxObject->RowCountChanged(aIndex + 1, -count);
  }
}

PRInt32
nsTreeContentView::FindContent(nsIContent* aContent)
{
  for (PRInt32 i = 0; i < mRows.Count(); i++) {
    if (((Row*)mRows[i])->mContent == aContent) {
      return i;
    }
  }

  return -1;
}

void
nsTreeContentView::UpdateSubtreeSizes(PRInt32 aParentIndex, PRInt32 count)
{
  while (aParentIndex >= 0) {
    Row* row = (Row*)mRows[aParentIndex];
    row->mSubtreeSize += count;
    aParentIndex = row->mParentIndex;
  }
}

void
nsTreeContentView::UpdateParentIndexes(PRInt32 aIndex, PRInt32 aSkip, PRInt32 aCount)
{
  PRInt32 count = mRows.Count();
  for (PRInt32 i = aIndex + aSkip; i < count; i++) {
    Row* row = (Row*)mRows[i];
    if (row->mParentIndex > aIndex) {
      row->mParentIndex += aCount;
    }
  }
}

nsIContent*
nsTreeContentView::GetCell(nsIContent* aContainer, nsITreeColumn* aCol)
{
  nsCOMPtr<nsIAtom> colAtom;
  PRInt32 colIndex;
  aCol->GetAtom(getter_AddRefs(colAtom));
  aCol->GetIndex(&colIndex);

  // Traverse through cells, try to find the cell by "ref" attribute or by cell
  // index in a row. "ref" attribute has higher priority.
  nsIContent* result = nsnull;
  PRInt32 j = 0;
  ChildIterator iter, last;
  for (ChildIterator::Init(aContainer, &iter, &last); iter != last; ++iter) {
    nsCOMPtr<nsIContent> cell = *iter;

    if (cell->Tag() == nsGkAtoms::treecell) {
      if (colAtom && cell->AttrValueIs(kNameSpaceID_None, nsGkAtoms::ref,
                                       colAtom, eCaseMatters)) {
        result = cell;
        break;
      }
      else if (j == colIndex) {
        result = cell;
      }
      j++;
    }
  }

  return result;
}
