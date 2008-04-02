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
 *   Dave Hyatt <hyatt@mozilla.org> (Original Author)
 *   Brian Ryner <bryner@brianryner.com>
 *   Nate Nielsen <nielsen@memberwebs.com>
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

#include "nsTreeBoxObject.h"
#include "nsCOMPtr.h"
#include "nsIDOMXULElement.h"
#include "nsIXULTemplateBuilder.h"
#include "nsTreeContentView.h"
#include "nsITreeSelection.h"
#include "nsChildIterator.h"
#include "nsContentUtils.h"
#include "nsDOMError.h"
#include "nsTreeBodyFrame.h"

NS_IMPL_ISUPPORTS_INHERITED1(nsTreeBoxObject, nsBoxObject, nsITreeBoxObject)

void
nsTreeBoxObject::Clear()
{
  ClearCachedValues();

  // Drop the view's ref to us.
  if (mView) {
    nsCOMPtr<nsITreeSelection> sel;
    mView->GetSelection(getter_AddRefs(sel));
    if (sel)
      sel->SetTree(nsnull);
    mView->SetTree(nsnull); // Break the circular ref between the view and us.
  }
  mView = nsnull;

  nsBoxObject::Clear();
}


nsTreeBoxObject::nsTreeBoxObject()
  : mTreeBody(nsnull)
{
}

nsTreeBoxObject::~nsTreeBoxObject()
{
  /* destructor code */
}


static void FindBodyElement(nsIContent* aParent, nsIContent** aResult)
{
  *aResult = nsnull;
  ChildIterator iter, last;
  for (ChildIterator::Init(aParent, &iter, &last); iter != last; ++iter) {
    nsCOMPtr<nsIContent> content = *iter;

    nsINodeInfo *ni = content->NodeInfo();
    if (ni->Equals(nsGkAtoms::treechildren, kNameSpaceID_XUL)) {
      *aResult = content;
      NS_ADDREF(*aResult);
      break;
    } else if (ni->Equals(nsGkAtoms::tree, kNameSpaceID_XUL)) {
      // There are nesting tree elements. Only the innermost should
      // find the treechilren.
      break;
    } else if (content->IsNodeOfType(nsINode::eELEMENT) &&
               !ni->Equals(nsGkAtoms::_template, kNameSpaceID_XUL)) {
      FindBodyElement(content, aResult);
      if (*aResult)
        break;
    }
  }
}

nsITreeBoxObject*
nsTreeBoxObject::GetTreeBody()
{
  if (mTreeBody) {
    return mTreeBody;
  }

  nsIFrame* frame = GetFrame(PR_FALSE);
  if (!frame)
    return nsnull;

  // Iterate over our content model children looking for the body.
  nsCOMPtr<nsIContent> content;
  FindBodyElement(frame->GetContent(), getter_AddRefs(content));

  nsIPresShell* shell = GetPresShell(PR_FALSE);
  if (!shell) {
    return nsnull;
  }

  frame = shell->GetPrimaryFrameFor(content);
  if (!frame)
     return nsnull;

  // It's a frame. Refcounts are irrelevant.
  // Make sure that the treebodyframe, which implements nsITreeBoxObject,
  // has a pointer to |this|.
  nsITreeBoxObject* innerTreeBoxObject = nsnull;
  CallQueryInterface(frame, &innerTreeBoxObject);
  NS_ENSURE_TRUE(innerTreeBoxObject &&
    static_cast<nsTreeBodyFrame*>(innerTreeBoxObject)->GetTreeBoxObject() ==
    static_cast<nsITreeBoxObject*>(this), nsnull);

  mTreeBody = innerTreeBoxObject;
  return mTreeBody;
}

NS_IMETHODIMP nsTreeBoxObject::GetView(nsITreeView * *aView)
{
  if (!mTreeBody) {
    if (!GetTreeBody()) {
      // Don't return an uninitialised view
      *aView = nsnull;
      return NS_OK;
    }

    if (mView)
      // Our new frame needs to initialise itself
      return mTreeBody->GetView(aView);
  }
  if (!mView) {
    nsCOMPtr<nsIDOMXULElement> xulele = do_QueryInterface(mContent);
    if (xulele) {
      // See if there is a XUL tree builder associated with the element
      nsCOMPtr<nsIXULTemplateBuilder> builder;
      xulele->GetBuilder(getter_AddRefs(builder));
      mView = do_QueryInterface(builder);

      if (!mView) {
        // No tree builder, create a tree content view.
        nsresult rv = NS_NewTreeContentView(getter_AddRefs(mView));
        NS_ENSURE_SUCCESS(rv, rv);
      }

      // Initialise the frame and view
      mTreeBody->SetView(mView);
    }
  }
  NS_IF_ADDREF(*aView = mView);
  return NS_OK;
}

static PRBool
CanTrustView(nsISupports* aValue)
{
  // Untrusted content is only allowed to specify known-good views
  if (nsContentUtils::IsCallerTrustedForWrite())
    return PR_TRUE;
  nsCOMPtr<nsINativeTreeView> nativeTreeView = do_QueryInterface(aValue);
  if (!nativeTreeView || NS_FAILED(nativeTreeView->EnsureNative())) {
    // XXX ERRMSG need a good error here for developers
    return PR_FALSE;
  }
  return PR_TRUE;
}

NS_IMETHODIMP nsTreeBoxObject::SetView(nsITreeView * aView)
{
  if (!CanTrustView(aView))
    return NS_ERROR_DOM_SECURITY_ERR;
  
  mView = aView;
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    body->SetView(aView);

  return NS_OK;
}

NS_IMETHODIMP nsTreeBoxObject::GetFocused(PRBool* aFocused)
{
  *aFocused = PR_FALSE;
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->GetFocused(aFocused);
  return NS_OK;
}

NS_IMETHODIMP nsTreeBoxObject::SetFocused(PRBool aFocused)
{
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->SetFocused(aFocused);
  return NS_OK;
}

NS_IMETHODIMP nsTreeBoxObject::GetTreeBody(nsIDOMElement** aElement)
{
  *aElement = nsnull;
  nsITreeBoxObject* body = GetTreeBody();
  if (body) 
    return body->GetTreeBody(aElement);
  return NS_OK;
}

NS_IMETHODIMP nsTreeBoxObject::GetColumns(nsITreeColumns** aColumns)
{
  *aColumns = nsnull;
  nsITreeBoxObject* body = GetTreeBody();
  if (body) 
    return body->GetColumns(aColumns);
  return NS_OK;
}

NS_IMETHODIMP nsTreeBoxObject::GetRowHeight(PRInt32* aRowHeight)
{
  *aRowHeight = 0;
  nsITreeBoxObject* body = GetTreeBody();
  if (body) 
    return body->GetRowHeight(aRowHeight);
  return NS_OK;
}

NS_IMETHODIMP nsTreeBoxObject::GetRowWidth(PRInt32 *aRowWidth)
{
  *aRowWidth = 0;
  nsITreeBoxObject* body = GetTreeBody();
  if (body) 
    return body->GetRowWidth(aRowWidth);
  return NS_OK;
}

NS_IMETHODIMP nsTreeBoxObject::GetFirstVisibleRow(PRInt32 *aFirstVisibleRow)
{
  *aFirstVisibleRow = 0;
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->GetFirstVisibleRow(aFirstVisibleRow);
  return NS_OK;
}

NS_IMETHODIMP nsTreeBoxObject::GetLastVisibleRow(PRInt32 *aLastVisibleRow)
{
  *aLastVisibleRow = 0;
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->GetLastVisibleRow(aLastVisibleRow);
  return NS_OK;
}

NS_IMETHODIMP nsTreeBoxObject::GetHorizontalPosition(PRInt32 *aHorizontalPosition)
{
  *aHorizontalPosition = 0;
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->GetHorizontalPosition(aHorizontalPosition);
  return NS_OK;
}

NS_IMETHODIMP nsTreeBoxObject::GetPageLength(PRInt32 *aPageLength)
{
  *aPageLength = 0;
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->GetPageLength(aPageLength);
  return NS_OK;
}

NS_IMETHODIMP
nsTreeBoxObject::EnsureRowIsVisible(PRInt32 aRow)
{
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->EnsureRowIsVisible(aRow);
  return NS_OK;
}

NS_IMETHODIMP 
nsTreeBoxObject::EnsureCellIsVisible(PRInt32 aRow, nsITreeColumn* aCol)
{
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->EnsureCellIsVisible(aRow, aCol);
  return NS_OK;
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsTreeBoxObject::ScrollToRow(PRInt32 aRow)
{
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->ScrollToRow(aRow);
  return NS_OK;
}

NS_IMETHODIMP
nsTreeBoxObject::ScrollByLines(PRInt32 aNumLines)
{
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->ScrollByLines(aNumLines);
  return NS_OK;
}

NS_IMETHODIMP
nsTreeBoxObject::ScrollByPages(PRInt32 aNumPages)
{
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->ScrollByPages(aNumPages);
  return NS_OK;
}

NS_IMETHODIMP 
nsTreeBoxObject::ScrollToCell(PRInt32 aRow, nsITreeColumn* aCol)
{
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->ScrollToCell(aRow, aCol);
  return NS_OK;
}

NS_IMETHODIMP 
nsTreeBoxObject::ScrollToColumn(nsITreeColumn* aCol)
{
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->ScrollToColumn(aCol);
  return NS_OK;
}

NS_IMETHODIMP 
nsTreeBoxObject::ScrollToHorizontalPosition(PRInt32 aHorizontalPosition)
{
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->ScrollToHorizontalPosition(aHorizontalPosition);
  return NS_OK;
}

NS_IMETHODIMP nsTreeBoxObject::Invalidate()
{
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->Invalidate();
  return NS_OK;
}

NS_IMETHODIMP nsTreeBoxObject::InvalidateColumn(nsITreeColumn* aCol)
{
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->InvalidateColumn(aCol);
  return NS_OK;
}

NS_IMETHODIMP nsTreeBoxObject::InvalidateRow(PRInt32 aIndex)
{
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->InvalidateRow(aIndex);
  return NS_OK;
}

NS_IMETHODIMP nsTreeBoxObject::InvalidateCell(PRInt32 aRow, nsITreeColumn* aCol)
{
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->InvalidateCell(aRow, aCol);
  return NS_OK;
}

NS_IMETHODIMP nsTreeBoxObject::InvalidateRange(PRInt32 aStart, PRInt32 aEnd)
{
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->InvalidateRange(aStart, aEnd);
  return NS_OK;
}

NS_IMETHODIMP nsTreeBoxObject::InvalidateColumnRange(PRInt32 aStart, PRInt32 aEnd, nsITreeColumn* aCol)
{
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->InvalidateColumnRange(aStart, aEnd, aCol);
  return NS_OK;
}

NS_IMETHODIMP nsTreeBoxObject::GetRowAt(PRInt32 x, PRInt32 y, PRInt32 *aRow)
{
  *aRow = 0;
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->GetRowAt(x, y, aRow);
  return NS_OK;
}

NS_IMETHODIMP nsTreeBoxObject::GetCellAt(PRInt32 aX, PRInt32 aY, PRInt32 *aRow, nsITreeColumn** aCol,
                                         nsACString& aChildElt)
{
  *aRow = 0;
  *aCol = nsnull;
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->GetCellAt(aX, aY, aRow, aCol, aChildElt);
  return NS_OK;
}

NS_IMETHODIMP
nsTreeBoxObject::GetCoordsForCellItem(PRInt32 aRow, nsITreeColumn* aCol, const nsACString& aElement, 
                                      PRInt32 *aX, PRInt32 *aY, PRInt32 *aWidth, PRInt32 *aHeight)
{
  *aX = *aY = *aWidth = *aHeight = 0;
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->GetCoordsForCellItem(aRow, aCol, aElement, aX, aY, aWidth, aHeight);
  return NS_OK;
}

NS_IMETHODIMP
nsTreeBoxObject::IsCellCropped(PRInt32 aRow, nsITreeColumn* aCol, PRBool *aIsCropped)
{  
  *aIsCropped = PR_FALSE;
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->IsCellCropped(aRow, aCol, aIsCropped);
  return NS_OK;
}

NS_IMETHODIMP nsTreeBoxObject::RowCountChanged(PRInt32 aIndex, PRInt32 aDelta)
{
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->RowCountChanged(aIndex, aDelta);
  return NS_OK;
}

NS_IMETHODIMP nsTreeBoxObject::BeginUpdateBatch()
{
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->BeginUpdateBatch();
  return NS_OK;
}

NS_IMETHODIMP nsTreeBoxObject::EndUpdateBatch()
{
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->EndUpdateBatch();
  return NS_OK;
}

NS_IMETHODIMP nsTreeBoxObject::ClearStyleAndImageCaches()
{
  nsITreeBoxObject* body = GetTreeBody();
  if (body)
    return body->ClearStyleAndImageCaches();
  return NS_OK;
}

void
nsTreeBoxObject::ClearCachedValues()
{
  mTreeBody = nsnull;
}    

// Creation Routine ///////////////////////////////////////////////////////////////////////

nsresult
NS_NewTreeBoxObject(nsIBoxObject** aResult)
{
  *aResult = new nsTreeBoxObject;
  if (!*aResult)
    return NS_ERROR_OUT_OF_MEMORY;
  NS_ADDREF(*aResult);
  return NS_OK;
}

