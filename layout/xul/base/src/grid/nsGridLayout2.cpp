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

//
// Eric Vaughan
// Netscape Communications
//
// See documentation in associated header file
//

#include "nsGridLayout2.h"
#include "nsGridRowGroupLayout.h"
#include "nsBox.h"
#include "nsIScrollableFrame.h"
#include "nsSprocketLayout.h"

nsresult
NS_NewGridLayout2( nsIPresShell* aPresShell, nsIBoxLayout** aNewLayout)
{
  *aNewLayout = new nsGridLayout2(aPresShell);
  NS_IF_ADDREF(*aNewLayout);

  return NS_OK;
  
} 

nsGridLayout2::nsGridLayout2(nsIPresShell* aPresShell):nsStackLayout()
{
}

NS_IMETHODIMP
nsGridLayout2::Layout(nsIBox* aBox, nsBoxLayoutState& aBoxLayoutState)
{
  // XXX This should be set a better way!
  mGrid.SetBox(aBox);
#ifdef DEBUG
  {
    nsCOMPtr<nsIBoxLayout> lm;
    aBox->GetLayoutManager(getter_AddRefs(lm));
    NS_ASSERTION(lm == this, "setting incorrect box");
  }
#endif

  nsresult rv = nsStackLayout::Layout(aBox, aBoxLayoutState);
#ifdef DEBUG_grid
  mGrid.PrintCellMap();
#endif
  return rv;
}

NS_IMETHODIMP
nsGridLayout2::GetGrid(nsIBox* aBox, nsGrid** aGrid, PRInt32* aIndex, nsGridRowLayout* aRequestor)
{
  // XXX This should be set a better way!
  mGrid.SetBox(aBox);
#ifdef DEBUG
  {
    nsCOMPtr<nsIBoxLayout> lm;
    aBox->GetLayoutManager(getter_AddRefs(lm));
    NS_ASSERTION(lm == this, "setting incorrect box");
  }
#endif

  *aGrid = &mGrid;
  return NS_OK;
}

NS_IMETHODIMP
nsGridLayout2::GetParentGridPart(nsIBox* aBox, nsIBox** aParentBox, nsIGridPart** aParentGridRow)
{
  NS_ERROR("Should not be called");
  return NS_ERROR_FAILURE;
}

void
nsGridLayout2::AddWidth(nsSize& aSize, nscoord aSize2, PRBool aIsHorizontal)
{
  nscoord& size = GET_WIDTH(aSize, aIsHorizontal);

  if (size != NS_INTRINSICSIZE) {
    if (aSize2 == NS_INTRINSICSIZE)
      size = NS_INTRINSICSIZE;
    else
      size += aSize2;
  }
}

NS_IMETHODIMP
nsGridLayout2::GetMinSize(nsIBox* aBox, nsBoxLayoutState& aState, nsSize& aSize)
{
  nsresult rv = nsStackLayout::GetMinSize(aBox, aState, aSize); 
  if (NS_FAILED(rv))
    return rv;

  // if there are no <rows> tags that will sum up our columns,
  // sum up our columns here.
  nsSize total(0,0);
  nsIBox* rowBox = mGrid.GetRowBox();
  nsIBox* columnBox = mGrid.GetColumnBox();
  if (!rowBox || !columnBox) {
    if (!rowBox) {
      // max height is the sum of our rows
      PRInt32 rows = mGrid.GetRowCount();
      for (PRInt32 i=0; i < rows; i++)
      {
        nscoord size = 0;
        mGrid.GetMinRowHeight(aState, i, size, PR_TRUE); 
        AddWidth(total, size, PR_FALSE); // AddHeight
      }
    }

    if (!columnBox) {
      // max height is the sum of our rows
      PRInt32 columns = mGrid.GetColumnCount();
      for (PRInt32 i=0; i < columns; i++)
      {
        nscoord size = 0;
        mGrid.GetMinRowHeight(aState, i, size, PR_FALSE); // GetPrefRowWidth
        AddWidth(total, size, PR_TRUE); // AddWidth
      }
    }

    AddMargin(aBox, total);
    AddOffset(aState, aBox, total);
    AddLargestSize(aSize, total);
  }
  
  return rv;
}

NS_IMETHODIMP
nsGridLayout2::GetPrefSize(nsIBox* aBox, nsBoxLayoutState& aState, nsSize& aSize)
{
  nsresult rv = nsStackLayout::GetPrefSize(aBox, aState, aSize); 
  if (NS_FAILED(rv))
    return rv;

  // if there are no <rows> tags that will sum up our columns,
  // sum up our columns here.
  nsSize total(0,0);
  nsIBox* rowBox = mGrid.GetRowBox();
  nsIBox* columnBox = mGrid.GetColumnBox();
  if (!rowBox || !columnBox) {
    if (!rowBox) {
      // max height is the sum of our rows
      PRInt32 rows = mGrid.GetRowCount();
      for (PRInt32 i=0; i < rows; i++)
      {
        nscoord size = 0;
        mGrid.GetPrefRowHeight(aState, i, size, PR_TRUE); 
        AddWidth(total, size, PR_FALSE); // AddHeight
      }
    }

    if (!columnBox) {
      // max height is the sum of our rows
      PRInt32 columns = mGrid.GetColumnCount();
      for (PRInt32 i=0; i < columns; i++)
      {
        nscoord size = 0;
        mGrid.GetPrefRowHeight(aState, i, size, PR_FALSE); // GetPrefRowWidth
        AddWidth(total, size, PR_TRUE); // AddWidth
      }
    }

    AddMargin(aBox, total);
    AddOffset(aState, aBox, total);
    AddLargestSize(aSize, total);
  }

  return rv;
}

NS_IMETHODIMP
nsGridLayout2::GetMaxSize(nsIBox* aBox, nsBoxLayoutState& aState, nsSize& aSize)
{
  nsresult rv = nsStackLayout::GetMaxSize(aBox, aState, aSize); 
   if (NS_FAILED(rv))
    return rv;

  // if there are no <rows> tags that will sum up our columns,
  // sum up our columns here.
  nsSize total(NS_INTRINSICSIZE, NS_INTRINSICSIZE);
  nsIBox* rowBox = mGrid.GetRowBox();
  nsIBox* columnBox = mGrid.GetColumnBox();
  if (!rowBox || !columnBox) {
    if (!rowBox) {
      total.height = 0;
      // max height is the sum of our rows
      PRInt32 rows = mGrid.GetRowCount();
      for (PRInt32 i=0; i < rows; i++)
      {
        nscoord size = 0;
        mGrid.GetMaxRowHeight(aState, i, size, PR_TRUE); 
        AddWidth(total, size, PR_FALSE); // AddHeight
      }
    }

    if (!columnBox) {
      total.width = 0;
      // max height is the sum of our rows
      PRInt32 columns = mGrid.GetColumnCount();
      for (PRInt32 i=0; i < columns; i++)
      {
        nscoord size = 0;
        mGrid.GetMaxRowHeight(aState, i, size, PR_FALSE); // GetPrefRowWidth
        AddWidth(total, size, PR_TRUE); // AddWidth
      }
    }

    AddMargin(aBox, total);
    AddOffset(aState, aBox, total);
    AddSmallestSize(aSize, total);
  }

  return rv;
}

NS_IMETHODIMP
nsGridLayout2::CountRowsColumns(nsIBox* aRowBox, PRInt32& aRowCount, PRInt32& aComputedColumnCount)
{
  NS_ERROR("Should not be called");
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
nsGridLayout2::DirtyRows(nsIBox* aBox, nsBoxLayoutState& aState)
{
  NS_ERROR("Should not be called");
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
nsGridLayout2::BuildRows(nsIBox* aBox, nsGridRow* aRows, PRInt32* aCount)
{
  NS_ERROR("Should not be called");
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
nsGridLayout2::CastToRowGroupLayout(nsGridRowGroupLayout** aRowGroup)
{
  (*aRowGroup) = nsnull;
  return NS_OK;
}

NS_IMETHODIMP
nsGridLayout2::CastToGridLayout(nsGridLayout2** aGridLayout)
{
  (*aGridLayout) = this;
  return NS_OK;
}

NS_IMETHODIMP
nsGridLayout2::GetTotalMargin(nsIBox* aBox, nsMargin& aMargin, PRBool aIsHorizontal)
{
  aMargin.left = 0;
  aMargin.right = 0;
  aMargin.top = 0;
  aMargin.bottom = 0;

  return NS_OK;
}

NS_IMETHODIMP
nsGridLayout2::GetRowCount(PRInt32& aRowCount)
{
  NS_ERROR("Should not be called");
  return NS_OK;
}

NS_IMETHODIMP_(nsIGridPart::Type)
nsGridLayout2::GetType()
{
  return eGrid;
}

NS_IMETHODIMP
nsGridLayout2::ChildrenInserted(nsIBox* aBox, nsBoxLayoutState& aState,
                                nsIBox* aPrevBox, nsIBox* aChildList)
{
  mGrid.NeedsRebuild(aState);
  return NS_OK;
}

NS_IMETHODIMP
nsGridLayout2::ChildrenAppended(nsIBox* aBox, nsBoxLayoutState& aState,
                                nsIBox* aChildList)
{
  mGrid.NeedsRebuild(aState);
  return NS_OK;
}


NS_IMETHODIMP
nsGridLayout2::ChildrenRemoved(nsIBox* aBox, nsBoxLayoutState& aState,
                               nsIBox* aChildList)
{
  mGrid.NeedsRebuild(aState);
  return NS_OK;
}


NS_IMETHODIMP
nsGridLayout2::ChildrenSet(nsIBox* aBox, nsBoxLayoutState& aState,
                           nsIBox* aChildList)
{
  mGrid.NeedsRebuild(aState);
  return NS_OK;
}

NS_IMPL_ADDREF_INHERITED(nsGridLayout2, nsStackLayout)
NS_IMPL_RELEASE_INHERITED(nsGridLayout2, nsStackLayout)

NS_INTERFACE_MAP_BEGIN(nsGridLayout2)
  NS_INTERFACE_MAP_ENTRY(nsIGridPart)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIGridPart)
NS_INTERFACE_MAP_END_INHERITING(nsStackLayout)
