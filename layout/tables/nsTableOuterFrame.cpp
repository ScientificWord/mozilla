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
#include "nsTableOuterFrame.h"
#include "nsTableFrame.h"
#include "nsStyleContext.h"
#include "nsStyleConsts.h"
#include "nsPresContext.h"
#include "nsIRenderingContext.h"
#include "nsCSSRendering.h"
#include "nsIContent.h"
#include "prinrval.h"
#include "nsGkAtoms.h"
#include "nsHTMLParts.h"
#include "nsIPresShell.h"
#ifdef ACCESSIBILITY
#include "nsIAccessibilityService.h"
#endif
#include "nsIServiceManager.h"
#include "nsIDOMNode.h"
#include "nsDisplayList.h"
#include "nsLayoutUtils.h"

/* ----------- nsTableCaptionFrame ---------- */

#define NS_TABLE_FRAME_CAPTION_LIST_INDEX 0
#define NO_SIDE 100

// caption frame
nsTableCaptionFrame::nsTableCaptionFrame(nsStyleContext* aContext):
  nsBlockFrame(aContext)
{
  // shrink wrap 
  SetFlags(NS_BLOCK_SPACE_MGR);
}

nsTableCaptionFrame::~nsTableCaptionFrame()
{
}

nsIAtom*
nsTableCaptionFrame::GetType() const
{
  return nsGkAtoms::tableCaptionFrame;
}

/* virtual */ nscoord
nsTableOuterFrame::GetBaseline() const
{
  nsIFrame* kid = mFrames.FirstChild();
  if (!kid) {
    NS_NOTREACHED("no inner table");
    return nsHTMLContainerFrame::GetBaseline();
  }

  return kid->GetBaseline() + kid->GetPosition().y;
}

/* virtual */ nsSize
nsTableCaptionFrame::ComputeAutoSize(nsIRenderingContext *aRenderingContext,
                                     nsSize aCBSize, nscoord aAvailableWidth,
                                     nsSize aMargin, nsSize aBorder,
                                     nsSize aPadding, PRBool aShrinkWrap)
{
  nsSize result = nsBlockFrame::ComputeAutoSize(aRenderingContext, aCBSize,
                    aAvailableWidth, aMargin, aBorder, aPadding, aShrinkWrap);
  PRUint8 captionSide = GetStyleTableBorder()->mCaptionSide;
  if (captionSide == NS_STYLE_CAPTION_SIDE_LEFT ||
      captionSide == NS_STYLE_CAPTION_SIDE_RIGHT) {
    result.width = GetMinWidth(aRenderingContext);
  } else if (captionSide == NS_STYLE_CAPTION_SIDE_TOP ||
             captionSide == NS_STYLE_CAPTION_SIDE_BOTTOM) {
    // The outer frame constrains our available width to the width of
    // the table.  Grow if our min-width is bigger than that, but not
    // larger than the containing block width.  (It would really be nice
    // to transmit that information another way, so we could grow up to
    // the table's available width, but that's harder.)
    nscoord min = GetMinWidth(aRenderingContext);
    if (min > aCBSize.width)
      min = aCBSize.width;
    if (min > result.width)
      result.width = min;
  }
  return result;
}

NS_IMETHODIMP 
nsTableCaptionFrame::GetParentStyleContextFrame(nsPresContext* aPresContext,
                                                nsIFrame**      aProviderFrame,
                                                PRBool*         aIsChild)
{
  NS_PRECONDITION(mContent->GetParent(),
                  "How could we not have a parent here?");
    
  // The caption's style context parent is the inner frame, unless
  // it's anonymous.
  nsIFrame* outerFrame = GetParent();
  if (outerFrame && outerFrame->GetType() == nsGkAtoms::tableOuterFrame) {
    nsIFrame* innerFrame = outerFrame->GetFirstChild(nsnull);
    if (innerFrame) {
      *aProviderFrame =
        nsFrame::CorrectStyleParentFrame(innerFrame,
                                         GetStyleContext()->GetPseudoType());
      *aIsChild = PR_FALSE;
      return NS_OK;
    }
  }

  NS_NOTREACHED("Where is our inner table frame?");
  return nsBlockFrame::GetParentStyleContextFrame(aPresContext, aProviderFrame,
                                                  aIsChild);
}

#ifdef ACCESSIBILITY
NS_IMETHODIMP nsTableCaptionFrame::GetAccessible(nsIAccessible** aAccessible)
{
  *aAccessible = nsnull;
  if (!GetRect().IsEmpty()) {
    nsCOMPtr<nsIAccessibilityService> accService = do_GetService("@mozilla.org/accessibilityService;1");
    if (accService) {
      return accService->CreateHTMLCaptionAccessible(static_cast<nsIFrame*>(this), aAccessible);
    }
  }

  return NS_ERROR_FAILURE;
}
#endif

#ifdef NS_DEBUG
NS_IMETHODIMP
nsTableCaptionFrame::GetFrameName(nsAString& aResult) const
{
  return MakeFrameName(NS_LITERAL_STRING("Caption"), aResult);
}
#endif

nsIFrame* 
NS_NewTableCaptionFrame(nsIPresShell* aPresShell, nsStyleContext* aContext)
{
  return new (aPresShell) nsTableCaptionFrame(aContext);
}

/* ----------- nsTableOuterFrame ---------- */

NS_IMPL_ADDREF_INHERITED(nsTableOuterFrame, nsHTMLContainerFrame)
NS_IMPL_RELEASE_INHERITED(nsTableOuterFrame, nsHTMLContainerFrame)

nsTableOuterFrame::nsTableOuterFrame(nsStyleContext* aContext):
  nsHTMLContainerFrame(aContext)
{
}

nsTableOuterFrame::~nsTableOuterFrame()
{
}

NS_IMETHODIMP
nsTableOuterFrame::QueryInterface(const nsIID& aIID, void** aInstancePtr)
{
  NS_PRECONDITION(aInstancePtr, "null out param");

  if (aIID.Equals(NS_GET_IID(nsITableLayout))) {
    *aInstancePtr = static_cast<nsITableLayout*>(this);
    return NS_OK;
  }

  return nsHTMLContainerFrame::QueryInterface(aIID, aInstancePtr);
}

#ifdef ACCESSIBILITY
NS_IMETHODIMP nsTableOuterFrame::GetAccessible(nsIAccessible** aAccessible)
{
  nsCOMPtr<nsIAccessibilityService> accService = do_GetService("@mozilla.org/accessibilityService;1");

  if (accService) {
    return accService->CreateHTMLTableAccessible(static_cast<nsIFrame*>(this), aAccessible);
  }

  return NS_ERROR_FAILURE;
}
#endif

/* virtual */ PRBool
nsTableOuterFrame::IsContainingBlock() const
{
  return PR_FALSE;
}

NS_IMETHODIMP
nsTableOuterFrame::Init(
                   nsIContent*           aContent,
                   nsIFrame*             aParent,
                   nsIFrame*             aPrevInFlow)
{
  nsresult rv = nsHTMLContainerFrame::Init(aContent, aParent, aPrevInFlow);
  
  // record that children that are ignorable whitespace should be excluded 
  mState |= NS_FRAME_EXCLUDE_IGNORABLE_WHITESPACE;

  return rv;
}

void
nsTableOuterFrame::Destroy()
{
  mCaptionFrames.DestroyFrames();
  nsHTMLContainerFrame::Destroy();
}

nsIFrame*
nsTableOuterFrame::GetFirstChild(nsIAtom* aListName) const
{
  if (nsGkAtoms::captionList == aListName) {
    return mCaptionFrames.FirstChild();
  }
  if (!aListName) {
    return mFrames.FirstChild();
  }
  return nsnull;
}

nsIAtom*
nsTableOuterFrame::GetAdditionalChildListName(PRInt32 aIndex) const
{
  if (aIndex == NS_TABLE_FRAME_CAPTION_LIST_INDEX) {
    return nsGkAtoms::captionList;
  }
  return nsnull;
}

NS_IMETHODIMP 
nsTableOuterFrame::SetInitialChildList(nsIAtom*        aListName,
                                       nsIFrame*       aChildList)
{
  if (nsGkAtoms::captionList == aListName) {
    // the frame constructor already checked for table-caption display type
    mCaptionFrames.SetFrames(aChildList);
    mCaptionFrame  = mCaptionFrames.FirstChild();
  }
  else {
    NS_ASSERTION(!aListName, "wrong childlist");
    NS_ASSERTION(mFrames.IsEmpty(), "Frame leak!");
    mFrames.SetFrames(aChildList);
    mInnerTableFrame = nsnull;
    if (aChildList) {
      if (nsGkAtoms::tableFrame == aChildList->GetType()) {
        mInnerTableFrame = (nsTableFrame*)aChildList;
      }
      else {
        NS_ERROR("expected a table frame");
        return NS_ERROR_INVALID_ARG;
      }
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsTableOuterFrame::AppendFrames(nsIAtom*        aListName,
                                nsIFrame*       aFrameList)
{
  nsresult rv;

  // We only have two child frames: the inner table and a caption frame.
  // The inner frame is provided when we're initialized, and it cannot change
  if (nsGkAtoms::captionList == aListName) {
    NS_ASSERTION(!aFrameList ||
                 aFrameList->GetType() == nsGkAtoms::tableCaptionFrame,
                 "appending non-caption frame to captionList");
    mCaptionFrames.AppendFrames(this, aFrameList);
    mCaptionFrame = mCaptionFrames.FirstChild();
    rv = NS_OK;

    // Reflow the new caption frame. It's already marked dirty, so
    // just tell the pres shell.
    PresContext()->PresShell()->
      FrameNeedsReflow(this, nsIPresShell::eTreeChange,
                       NS_FRAME_HAS_DIRTY_CHILDREN);
  }
  else {
    NS_PRECONDITION(PR_FALSE, "unexpected child list");
    rv = NS_ERROR_UNEXPECTED;
  }

  return rv;
}

NS_IMETHODIMP
nsTableOuterFrame::InsertFrames(nsIAtom*        aListName,
                                nsIFrame*       aPrevFrame,
                                nsIFrame*       aFrameList)
{
  if (nsGkAtoms::captionList == aListName) {
    NS_ASSERTION(!aPrevFrame || aPrevFrame->GetParent() == this,
                 "inserting after sibling frame with different parent");
    NS_ASSERTION(!aFrameList ||
                 aFrameList->GetType() == nsGkAtoms::tableCaptionFrame,
                 "inserting non-caption frame into captionList");
    mCaptionFrames.InsertFrames(nsnull, aPrevFrame, aFrameList);
    mCaptionFrame = mCaptionFrames.FirstChild();

    // Reflow the new caption frame. It's already marked dirty, so
    // just tell the pres shell.
    PresContext()->PresShell()->
      FrameNeedsReflow(this, nsIPresShell::eTreeChange,
                       NS_FRAME_HAS_DIRTY_CHILDREN);
    return NS_OK;
  }
  else {
    NS_PRECONDITION(!aPrevFrame, "invalid previous frame");
    return AppendFrames(aListName, aFrameList);
  }
}

NS_IMETHODIMP
nsTableOuterFrame::RemoveFrame(nsIAtom*        aListName,
                               nsIFrame*       aOldFrame)
{
  // We only have two child frames: the inner table and one caption frame.
  // The inner frame can't be removed so this should be the caption
  NS_PRECONDITION(nsGkAtoms::captionList == aListName, "can't remove inner frame");

  if (HasSideCaption()) {
    // The old caption width had an effect on the inner table width so
    // we're going to need to reflow it. Mark it dirty
    mInnerTableFrame->AddStateBits(NS_FRAME_IS_DIRTY);
  }

  // Remove the frame and destroy it
  mCaptionFrames.DestroyFrame(aOldFrame);
  mCaptionFrame = mCaptionFrames.FirstChild();
  
  PresContext()->PresShell()->
    FrameNeedsReflow(this, nsIPresShell::eTreeChange,
                     NS_FRAME_HAS_DIRTY_CHILDREN); // also means child removed

  return NS_OK;
}

NS_METHOD 
nsTableOuterFrame::BuildDisplayList(nsDisplayListBuilder*   aBuilder,
                                    const nsRect&           aDirtyRect,
                                    const nsDisplayListSet& aLists)
{
  // No border, background or outline are painted because they all belong
  // to the inner table.
  if (!IsVisibleInSelection(aBuilder))
    return NS_OK;

  // If there's no caption, take a short cut to avoid having to create
  // the special display list set and then sort it.
  if (!mCaptionFrame)
    return BuildDisplayListForInnerTable(aBuilder, aDirtyRect, aLists);
    
  nsDisplayListCollection set;
  nsresult rv = BuildDisplayListForInnerTable(aBuilder, aDirtyRect, set);
  NS_ENSURE_SUCCESS(rv, rv);
  
  nsDisplayListSet captionSet(set, set.BlockBorderBackgrounds());
  rv = BuildDisplayListForChild(aBuilder, mCaptionFrame, aDirtyRect, captionSet);
  NS_ENSURE_SUCCESS(rv, rv);
  
  // Now we have to sort everything by content order, since the caption
  // may be somewhere inside the table
  set.SortAllByContentOrder(aBuilder, GetContent());
  set.MoveTo(aLists);
  return NS_OK;
}

nsresult
nsTableOuterFrame::BuildDisplayListForInnerTable(nsDisplayListBuilder*   aBuilder,
                                                 const nsRect&           aDirtyRect,
                                                 const nsDisplayListSet& aLists)
{
  // Just paint the regular children, but the children's background is our
  // true background (there should only be one, the real table)
  nsIFrame* kid = mFrames.FirstChild();
  // The children should be in content order
  while (kid) {
    nsresult rv = BuildDisplayListForChild(aBuilder, kid, aDirtyRect, aLists);
    NS_ENSURE_SUCCESS(rv, rv);
    kid = kid->GetNextSibling();
  }
  return NS_OK;
}

NS_IMETHODIMP nsTableOuterFrame::SetSelected(nsPresContext* aPresContext,
                                             nsIDOMRange *aRange,
                                             PRBool aSelected,
                                             nsSpread aSpread)
{
  nsresult result = nsFrame::SetSelected(aPresContext, aRange,aSelected, aSpread);
  if (NS_SUCCEEDED(result) && mInnerTableFrame)
    return mInnerTableFrame->SetSelected(aPresContext, aRange,aSelected, aSpread);
  return result;
}

NS_IMETHODIMP 
nsTableOuterFrame::GetParentStyleContextFrame(nsPresContext* aPresContext,
                                              nsIFrame**      aProviderFrame,
                                              PRBool*         aIsChild)
{
  // The table outer frame and the (inner) table frame split the style
  // data by giving the table frame the style context associated with
  // the table content node and creating a style context for the outer
  // frame that is a *child* of the table frame's style context,
  // matching the ::-moz-table-outer pseudo-element.  html.css has a
  // rule that causes that pseudo-element (and thus the outer table)
  // to inherit *some* style properties from the table frame.  The
  // children of the table inherit directly from the inner table, and
  // the outer table's style context is a leaf.

  if (!mInnerTableFrame) {
    *aProviderFrame = this;
    *aIsChild = PR_FALSE;
    return NS_ERROR_FAILURE;
  }
  *aProviderFrame = mInnerTableFrame;
  *aIsChild = PR_TRUE;
  return NS_OK;
}

// INCREMENTAL REFLOW HELPER FUNCTIONS 

void
nsTableOuterFrame::InitChildReflowState(nsPresContext&    aPresContext,                     
                                        nsHTMLReflowState& aReflowState)
                                    
{
  nsMargin collapseBorder;
  nsMargin collapsePadding(0,0,0,0);
  nsMargin* pCollapseBorder  = nsnull;
  nsMargin* pCollapsePadding = nsnull;
  if ((aReflowState.frame == mInnerTableFrame) && (mInnerTableFrame->IsBorderCollapse())) {
    collapseBorder  = mInnerTableFrame->GetIncludedOuterBCBorder();
    pCollapseBorder = &collapseBorder;
    pCollapsePadding = &collapsePadding;
  }
  aReflowState.Init(&aPresContext, -1, -1, pCollapseBorder, pCollapsePadding);
}

// get the margin and padding data. nsHTMLReflowState doesn't handle the
// case of auto margins
void
nsTableOuterFrame::GetMargin(nsPresContext*           aPresContext,
                             const nsHTMLReflowState& aOuterRS,
                             nsIFrame*                aChildFrame,
                             nscoord                  aAvailWidth,
                             nsMargin&                aMargin)
{
  // construct a reflow state to compute margin and padding. Auto margins
  // will not be computed at this time.

  // create and init the child reflow state
  // XXX We really shouldn't construct a reflow state to do this.
  nsHTMLReflowState childRS(aPresContext, aOuterRS, aChildFrame,
                            nsSize(aAvailWidth, aOuterRS.availableHeight),
                            -1, -1, PR_FALSE);
  InitChildReflowState(*aPresContext, childRS);

  aMargin = childRS.mComputedMargin;
}

static
nscoord CalcAutoMargin(nscoord aAutoMargin,
                       nscoord aOppositeMargin,
                       nscoord aContainBlockSize,
                       nscoord aFrameSize)
{
  nscoord margin;
  if (NS_AUTOMARGIN == aOppositeMargin) 
    margin = (aContainBlockSize - aFrameSize) / 2;
  else {
    margin = aContainBlockSize - aFrameSize - aOppositeMargin;
  }
  return PR_MAX(0, margin);
}

static nsSize
GetContainingBlockSize(const nsHTMLReflowState& aOuterRS)
{
  nsSize size(0,0);
  const nsHTMLReflowState* containRS =
    aOuterRS.mCBReflowState;

  if (containRS) {
    size.width = containRS->ComputedWidth();
    if (NS_UNCONSTRAINEDSIZE == size.width) {
      size.width = 0;
    }
    size.height = containRS->ComputedHeight();
    if (NS_UNCONSTRAINEDSIZE == size.height) {
      size.height = 0;
    }
  }
  return size;
}

void
nsTableOuterFrame::InvalidateDamage(PRUint8         aCaptionSide,
                                    const nsSize&   aOuterSize,
                                    PRBool          aInnerChanged,
                                    PRBool          aCaptionChanged,
                                    nsRect*         aOldOverflowArea)
{
  if (!aInnerChanged && !aCaptionChanged) return;

  nsRect damage;
  if (aInnerChanged && aCaptionChanged) {
    damage = nsRect(0, 0, aOuterSize.width, aOuterSize.height);
    if (aOldOverflowArea) {
      damage.UnionRect(damage, *aOldOverflowArea);
    }
    damage.UnionRect(damage, GetOverflowRect());
  }
  else {
    nsRect captionRect(0,0,0,0);
    nsRect innerRect = mInnerTableFrame->GetRect();
    if (mCaptionFrame) {
      captionRect = mCaptionFrame->GetRect();
    }
    
    damage.x = 0;
    damage.width  = aOuterSize.width;
    switch(aCaptionSide) {
    case NS_STYLE_CAPTION_SIDE_BOTTOM:
    case NS_STYLE_CAPTION_SIDE_BOTTOM_OUTSIDE:
      if (aCaptionChanged) {
        damage.y = innerRect.y;
        damage.height = aOuterSize.height - damage.y;
      }
      else { // aInnerChanged 
        damage.y = 0;
        damage.height = captionRect.y;
      }
      break;
    case NS_STYLE_CAPTION_SIDE_LEFT:
      if (aCaptionChanged) {
        damage.width = innerRect.x;
        damage.y = 0;
        damage.height = captionRect.YMost();
      }
      else { // aInnerChanged
        damage.x = captionRect.XMost();
        damage.width = innerRect.XMost() - damage.x;
        damage.y = 0;
        damage.height = innerRect.YMost();
      }
      break;
    case NS_STYLE_CAPTION_SIDE_RIGHT:
     if (aCaptionChanged) {
        damage.x = innerRect.XMost();
        damage.width -= damage.x;
        damage.y = 0;
        damage.height = captionRect.YMost();
      }
     else { // aInnerChanged
        damage.width -= captionRect.width;
        damage.y = 0;
        damage.height = innerRect.YMost();
      }
      break;
    default:
      NS_ASSERTION(aCaptionSide == NS_STYLE_CAPTION_SIDE_TOP ||
                   aCaptionSide == NS_STYLE_CAPTION_SIDE_TOP_OUTSIDE ||
                   aCaptionSide == NO_SIDE,
                   "unexpected caption side");
      if (aCaptionChanged) {
        damage.y = 0;
        damage.height = innerRect.y;
      }
      else { // aInnerChanged
        damage.y = captionRect.y;
        damage.height = aOuterSize.height - damage.y;
      }
      break;
    }
     
    nsIFrame* kidFrame = aCaptionChanged ? mCaptionFrame : mInnerTableFrame;
    ConsiderChildOverflow(damage, kidFrame);
    if (aOldOverflowArea) {
      damage.UnionRect(damage, *aOldOverflowArea);
    }
  }
  Invalidate(damage);
}

/* virtual */ nscoord
nsTableOuterFrame::GetMinWidth(nsIRenderingContext *aRenderingContext)
{
  nscoord width = nsLayoutUtils::IntrinsicForContainer(aRenderingContext,
                    mInnerTableFrame, nsLayoutUtils::MIN_WIDTH);
  DISPLAY_MIN_WIDTH(this, width);
  if (mCaptionFrame) {
    nscoord capWidth =
      nsLayoutUtils::IntrinsicForContainer(aRenderingContext, mCaptionFrame,
                                           nsLayoutUtils::MIN_WIDTH);
    if (HasSideCaption()) {
      width += capWidth;
    } else {
      if (capWidth > width) {
        width = capWidth;
      }
    }
  }
  return width;
}

/* virtual */ nscoord
nsTableOuterFrame::GetPrefWidth(nsIRenderingContext *aRenderingContext)
{
  nscoord maxWidth;
  DISPLAY_PREF_WIDTH(this, maxWidth);

  maxWidth = nsLayoutUtils::IntrinsicForContainer(aRenderingContext,
               mInnerTableFrame, nsLayoutUtils::PREF_WIDTH);
  if (mCaptionFrame) {
    PRUint8 captionSide = GetCaptionSide();
    switch(captionSide) {
    case NS_STYLE_CAPTION_SIDE_LEFT:
    case NS_STYLE_CAPTION_SIDE_RIGHT:
      {
        nscoord capMin =
          nsLayoutUtils::IntrinsicForContainer(aRenderingContext, mCaptionFrame,
                                               nsLayoutUtils::MIN_WIDTH);
        maxWidth += capMin;
      }
      break;
    default:
      {
        nsLayoutUtils::IntrinsicWidthType iwt;
        if (captionSide == NS_STYLE_CAPTION_SIDE_TOP ||
            captionSide == NS_STYLE_CAPTION_SIDE_BOTTOM) {
          // Don't let the caption's pref width expand the table's pref
          // width.
          iwt = nsLayoutUtils::MIN_WIDTH;
        } else {
          NS_ASSERTION(captionSide == NS_STYLE_CAPTION_SIDE_TOP_OUTSIDE ||
                       captionSide == NS_STYLE_CAPTION_SIDE_BOTTOM_OUTSIDE,
                       "unexpected caption side");
          iwt = nsLayoutUtils::PREF_WIDTH;
        }
        nscoord capPref =
          nsLayoutUtils::IntrinsicForContainer(aRenderingContext, mCaptionFrame,
                                               iwt);
        maxWidth = PR_MAX(maxWidth, capPref);
      }
      break;
    }
  }
  return maxWidth;
}

// Compute the margin-box width of aChildFrame given the inputs.  If
// aMarginResult is non-null, fill it with the part of the margin-width
// that was contributed by the margin.
static nscoord
ChildShrinkWrapWidth(nsIRenderingContext *aRenderingContext,
                     nsIFrame *aChildFrame,
                     nsSize aCBSize, nscoord aAvailableWidth,
                     nscoord *aMarginResult = nsnull)
{
  // The outer table's children do not use it as a containing block.
  nsCSSOffsetState offsets(aChildFrame, aRenderingContext, aCBSize.width);
  nsSize size = aChildFrame->ComputeSize(aRenderingContext, aCBSize,
                  aAvailableWidth,
                  nsSize(offsets.mComputedMargin.LeftRight(),
                         offsets.mComputedMargin.TopBottom()),
                  nsSize(offsets.mComputedBorderPadding.LeftRight() -
                           offsets.mComputedPadding.LeftRight(),
                         offsets.mComputedBorderPadding.TopBottom() -
                           offsets.mComputedPadding.TopBottom()),
                  nsSize(offsets.mComputedPadding.LeftRight(),
                         offsets.mComputedPadding.TopBottom()),
                  PR_TRUE);
  if (aMarginResult)
    *aMarginResult = offsets.mComputedMargin.LeftRight();
  return size.width + offsets.mComputedMargin.LeftRight() +
                      offsets.mComputedBorderPadding.LeftRight();
}

/* virtual */ nsSize
nsTableOuterFrame::ComputeAutoSize(nsIRenderingContext *aRenderingContext,
                                   nsSize aCBSize, nscoord aAvailableWidth,
                                   nsSize aMargin, nsSize aBorder,
                                   nsSize aPadding, PRBool aShrinkWrap)
{
  if (!aShrinkWrap)
    return nsHTMLContainerFrame::ComputeAutoSize(aRenderingContext, aCBSize,
               aAvailableWidth, aMargin, aBorder, aPadding, aShrinkWrap);

  // When we're shrink-wrapping, our auto size needs to wrap around the
  // actual size of the table, which (if it is specified as a percent)
  // could be something that is not reflected in our GetMinWidth and
  // GetPrefWidth.  See bug 349457 for an example.

  // Match the availableWidth logic in Reflow.
  PRUint8 captionSide = GetCaptionSide();
  nscoord width;
  if (captionSide == NO_SIDE) {
    width = ChildShrinkWrapWidth(aRenderingContext, mInnerTableFrame,
                                 aCBSize, aAvailableWidth);
  } else if (captionSide == NS_STYLE_CAPTION_SIDE_LEFT ||
             captionSide == NS_STYLE_CAPTION_SIDE_RIGHT) {
    nscoord capWidth = ChildShrinkWrapWidth(aRenderingContext, mCaptionFrame,
                                            aCBSize, aAvailableWidth);
    width = capWidth + ChildShrinkWrapWidth(aRenderingContext,
                                            mInnerTableFrame, aCBSize,
                                            aAvailableWidth - capWidth);
  } else if (captionSide == NS_STYLE_CAPTION_SIDE_TOP ||
             captionSide == NS_STYLE_CAPTION_SIDE_BOTTOM) {
    nscoord margin;
    width = ChildShrinkWrapWidth(aRenderingContext, mInnerTableFrame,
                                 aCBSize, aAvailableWidth, &margin);
    nscoord capWidth = ChildShrinkWrapWidth(aRenderingContext,
                                            mCaptionFrame, aCBSize,
                                            width - margin);
    if (capWidth > width)
      width = capWidth;
  } else {
    NS_ASSERTION(captionSide == NS_STYLE_CAPTION_SIDE_TOP_OUTSIDE ||
                 captionSide == NS_STYLE_CAPTION_SIDE_BOTTOM_OUTSIDE,
                 "unexpected caption-side");
    width = ChildShrinkWrapWidth(aRenderingContext, mInnerTableFrame,
                                 aCBSize, aAvailableWidth);
    nscoord capWidth = ChildShrinkWrapWidth(aRenderingContext,
                                            mCaptionFrame, aCBSize,
                                            aAvailableWidth);
    if (capWidth > width)
      width = capWidth;
  }

  return nsSize(width, NS_UNCONSTRAINEDSIZE);
}

PRUint8
nsTableOuterFrame::GetCaptionSide()
{
  if (mCaptionFrame) {
    return mCaptionFrame->GetStyleTableBorder()->mCaptionSide;
  }
  else {
    return NO_SIDE; // no caption
  }
}

PRUint8
nsTableOuterFrame::GetCaptionVerticalAlign()
{
  const nsStyleCoord& va = mCaptionFrame->GetStyleTextReset()->mVerticalAlign;
  return (va.GetUnit() == eStyleUnit_Enumerated)
           ? va.GetIntValue()
           : NS_STYLE_VERTICAL_ALIGN_TOP;
}

void
nsTableOuterFrame::SetDesiredSize(PRUint8         aCaptionSide,
                                  const nsMargin& aInnerMargin,
                                  const nsMargin& aCaptionMargin,
                                  nscoord&        aWidth,
                                  nscoord&        aHeight)
{
  aWidth = aHeight = 0;

  nsRect innerRect = mInnerTableFrame->GetRect();
  nscoord innerWidth = innerRect.width;

  nsRect captionRect(0,0,0,0);
  nscoord captionWidth = 0;
  if (mCaptionFrame) {
    captionRect = mCaptionFrame->GetRect();
    captionWidth = captionRect.width;
  }
  switch(aCaptionSide) {
    case NS_STYLE_CAPTION_SIDE_LEFT:
      aWidth = PR_MAX(aInnerMargin.left, aCaptionMargin.left + captionWidth + aCaptionMargin.right) +
               innerWidth + aInnerMargin.right;
      break;
    case NS_STYLE_CAPTION_SIDE_RIGHT:
      aWidth = PR_MAX(aInnerMargin.right, aCaptionMargin.left + captionWidth + aCaptionMargin.right) +
               innerWidth + aInnerMargin.left;
      break;
    default:
      aWidth = aInnerMargin.left + innerWidth + aInnerMargin.right;
      aWidth = PR_MAX(aWidth, captionRect.XMost() + aCaptionMargin.right);
  }
  aHeight = innerRect.YMost() + aInnerMargin.bottom;
  aHeight = PR_MAX(aHeight, captionRect.YMost() + aCaptionMargin.bottom);

}

// XXX This is now unused, but it probably should be used!
void
nsTableOuterFrame::BalanceLeftRightCaption(PRUint8         aCaptionSide,
                                           const nsMargin& aInnerMargin,
                                           const nsMargin& aCaptionMargin,
                                           nscoord&        aInnerWidth, 
                                           nscoord&        aCaptionWidth)
{
  
  /* balance the caption and inner table widths to ensure space for percent widths
  *  Percent widths for captions or the inner table frame can determine how much of the
  *  available width is used and how the available width is distributed between those frames
  *  The inner table frame has already a quite sophisticated treatment of percentage widths 
  *  (see BasicTableLayoutStrategy.cpp). So it acts as master in the below computations.
  *  There are four possible scenarios 
  *  a) None of the frames have a percentage width - then the aInnerWidth and aCaptionwidth will not change
  *  b) Only the inner frame has a percentage width - this is handled in BasicTableLayoutStrategy.cpp, 
  *     both widths will not change
  *  c) Only the caption has a percentage width - then the overall width (ow) will be different depending on
  *     the caption side. For the left side
  *     ow = aCaptionMargin.left + aCaptionWidth + aCaptionMargin.right + aInnerwidth + aInnerMargin.right
  *     aCaptionWidth = capPercent * ow
  *     solving this equation for aCaptionWidth gives:
  *     aCaptionWidth = capPercent/(1-capPercent) * 
  *                      (aCaptionMargin.left + aCaptionMargin.right + aInnerwidth + aInnerMargin.right)
  *     this result will cause problems for capPercent >= 1, in these cases the algorithm will now bail out
  *     a similar expression can be found for the right case
  *  d) both frames have percent widths in this case the caption width will be the inner width multiplied 
  *     by the weight capPercent/innerPercent
  */
    

  float capPercent   = -1.0;
  float innerPercent = -1.0;
  const nsStylePosition* position = mCaptionFrame->GetStylePosition();
  if (eStyleUnit_Percent == position->mWidth.GetUnit()) {
    capPercent = position->mWidth.GetPercentValue();
    if (capPercent >= 1.0)
      return;
  }

  position = mInnerTableFrame->GetStylePosition();
  if (eStyleUnit_Percent == position->mWidth.GetUnit()) {
    innerPercent = position->mWidth.GetPercentValue();
    if (innerPercent >= 1.0)
      return;
  }

  if ((capPercent <= 0.0) && (innerPercent <= 0.0))
    return;

  
  if (innerPercent <= 0.0) {
    if (NS_STYLE_CAPTION_SIDE_LEFT == aCaptionSide) 
      aCaptionWidth= (nscoord) ((capPercent / (1.0 - capPercent)) * (aCaptionMargin.left + aCaptionMargin.right + 
                                                          aInnerWidth + aInnerMargin.right));
    else
      aCaptionWidth= (nscoord) ((capPercent / (1.0 - capPercent)) * (aCaptionMargin.left + aCaptionMargin.right + 
                                                          aInnerWidth + aInnerMargin.left)); 
  } 
  else {
    aCaptionWidth = (nscoord) ((capPercent / innerPercent) * aInnerWidth);
  }
}

nsresult 
nsTableOuterFrame::GetCaptionOrigin(PRUint32         aCaptionSide,
                                    const nsSize&    aContainBlockSize,
                                    const nsSize&    aInnerSize, 
                                    const nsMargin&  aInnerMargin,
                                    const nsSize&    aCaptionSize,
                                    nsMargin&        aCaptionMargin,
                                    nsPoint&         aOrigin)
{
  // FIXME: This function expects computed margin values to be
  // NS_AUTOMARGIN, but this is no longer the case.
  aOrigin.x = aOrigin.y = 0;
  if ((NS_UNCONSTRAINEDSIZE == aInnerSize.width) || (NS_UNCONSTRAINEDSIZE == aInnerSize.height) ||  
      (NS_UNCONSTRAINEDSIZE == aCaptionSize.width) || (NS_UNCONSTRAINEDSIZE == aCaptionSize.height)) {
    return NS_OK;
  }
  if (!mCaptionFrame) return NS_OK;

  // FIXME: Have two separate switch statements so we can coalesce the
  // horizontal computation for top and bottom.
  switch(aCaptionSide) {
  case NS_STYLE_CAPTION_SIDE_BOTTOM:
  case NS_STYLE_CAPTION_SIDE_BOTTOM_OUTSIDE: {
    if (NS_AUTOMARGIN == aCaptionMargin.left) {
      aCaptionMargin.left = CalcAutoMargin(aCaptionMargin.left, aCaptionMargin.right,
                                           aContainBlockSize.width, aCaptionSize.width);
    }
    aOrigin.x = aCaptionMargin.left;
    if (aCaptionSide == NS_STYLE_CAPTION_SIDE_BOTTOM) {
      // We placed the caption using only the table's width as available
      // width, and we should position it this way as well.
      aOrigin.x += aInnerMargin.left;
    }
    if (NS_AUTOMARGIN == aCaptionMargin.top) {
      aCaptionMargin.top = 0;
    }
    // FIXME: Position relative to right edge for RTL.  (Based on table
    // direction or table parent direction?)
    nsCollapsingMargin marg;
    marg.Include(aCaptionMargin.top);
    marg.Include(aInnerMargin.bottom);
    nscoord collapseMargin = marg.get();
    if (NS_AUTOMARGIN == aCaptionMargin.bottom) {
      nscoord height = aInnerSize.height + collapseMargin + aCaptionSize.height;
      aCaptionMargin.bottom = CalcAutoMargin(aCaptionMargin.bottom, aInnerMargin.top,
                                             aContainBlockSize.height, height);
    }
    aOrigin.y = aInnerMargin.top + aInnerSize.height + collapseMargin;
  } break;
  case NS_STYLE_CAPTION_SIDE_LEFT: {
    if (NS_AUTOMARGIN == aCaptionMargin.left) {
      if (NS_AUTOMARGIN != aInnerMargin.left) {
        aCaptionMargin.left = CalcAutoMargin(aCaptionMargin.left, aCaptionMargin.right,
                                             aInnerMargin.left, aCaptionSize.width);
      } 
      else {
        // zero for now
        aCaptionMargin.left = 0;
      } 
    }
    aOrigin.x = aCaptionMargin.left;
    aOrigin.y = aInnerMargin.top;
    switch(GetCaptionVerticalAlign()) {
      case NS_STYLE_VERTICAL_ALIGN_MIDDLE:
        aOrigin.y = PR_MAX(0, aInnerMargin.top + ((aInnerSize.height - aCaptionSize.height) / 2));
        break;
      case NS_STYLE_VERTICAL_ALIGN_BOTTOM:
        aOrigin.y = PR_MAX(0, aInnerMargin.top + aInnerSize.height - aCaptionSize.height);
        break;
      default:
        break;
    }
  } break;
  case NS_STYLE_CAPTION_SIDE_RIGHT: {
    if (NS_AUTOMARGIN == aCaptionMargin.left) {
      if (NS_AUTOMARGIN != aInnerMargin.right) {
        aCaptionMargin.left = CalcAutoMargin(aCaptionMargin.left, aCaptionMargin.right,
                                             aInnerMargin.right, aCaptionSize.width);
      }
      else {
       // zero for now
       aCaptionMargin.left = 0;
      } 
    }
    aOrigin.x = aInnerMargin.left + aInnerSize.width + aCaptionMargin.left;
    aOrigin.y = aInnerMargin.top;
    switch(GetCaptionVerticalAlign()) {
      case NS_STYLE_VERTICAL_ALIGN_MIDDLE:
        aOrigin.y += PR_MAX(0, (aInnerSize.height - aCaptionSize.height) / 2);
        break;
      case NS_STYLE_VERTICAL_ALIGN_BOTTOM:
        aOrigin.y += PR_MAX(0, aInnerSize.height - aCaptionSize.height);
        break;
      default:
        break;
    }
  } break;
  default: { // top
    NS_ASSERTION(aCaptionSide == NS_STYLE_CAPTION_SIDE_TOP ||
                 aCaptionSide == NS_STYLE_CAPTION_SIDE_TOP_OUTSIDE,
                 "unexpected caption side");
    if (NS_AUTOMARGIN == aCaptionMargin.left) {
      aCaptionMargin.left = CalcAutoMargin(aCaptionMargin.left, aCaptionMargin.right,
                                           aContainBlockSize.width, aCaptionSize.width);
    }
    aOrigin.x = aCaptionMargin.left;
    if (aCaptionSide == NS_STYLE_CAPTION_SIDE_TOP) {
      // We placed the caption using only the table's width as available
      // width, and we should position it this way as well.
      aOrigin.x += aInnerMargin.left;
    }
    // FIXME: Position relative to right edge for RTL.  (Based on table
    // direction or table parent direction?)
    if (NS_AUTOMARGIN == aCaptionMargin.bottom) {
      aCaptionMargin.bottom = 0;
    }
    if (NS_AUTOMARGIN == aCaptionMargin.top) {
      nsCollapsingMargin marg;
      marg.Include(aCaptionMargin.bottom);
      marg.Include(aInnerMargin.top);
      nscoord collapseMargin = marg.get();
      nscoord height = aCaptionSize.height + collapseMargin + aInnerSize.height;
      aCaptionMargin.top = CalcAutoMargin(aCaptionMargin.top, aInnerMargin.bottom,
                                          aContainBlockSize.height, height);
    }
    aOrigin.y = aCaptionMargin.top;
  } break;
  }
  return NS_OK;
}

nsresult 
nsTableOuterFrame::GetInnerOrigin(PRUint32         aCaptionSide,
                                  const nsSize&    aContainBlockSize,
                                  const nsSize&    aCaptionSize, 
                                  const nsMargin&  aCaptionMargin,
                                  const nsSize&    aInnerSize,
                                  nsMargin&        aInnerMargin,
                                  nsPoint&         aOrigin)
{
  // FIXME: This function expects computed margin values to be
  // NS_AUTOMARGIN, but this is no longer the case.
  aOrigin.x = aOrigin.y = 0;
  if ((NS_UNCONSTRAINEDSIZE == aInnerSize.width) || (NS_UNCONSTRAINEDSIZE == aInnerSize.height) ||  
      (NS_UNCONSTRAINEDSIZE == aCaptionSize.width) || (NS_UNCONSTRAINEDSIZE == aCaptionSize.height)) {
    return NS_OK;
  }

  nscoord minCapWidth = aCaptionSize.width;
  if (NS_AUTOMARGIN != aCaptionMargin.left)
    minCapWidth += aCaptionMargin.left;
  if (NS_AUTOMARGIN != aCaptionMargin.right)
    minCapWidth += aCaptionMargin.right;

  switch(aCaptionSide) {
  case NS_STYLE_CAPTION_SIDE_BOTTOM:
  case NS_STYLE_CAPTION_SIDE_BOTTOM_OUTSIDE: {
    if (NS_AUTOMARGIN == aInnerMargin.left) {
      aInnerMargin.left = CalcAutoMargin(aInnerMargin.left, aInnerMargin.right,
                                         aContainBlockSize.width, aInnerSize.width);
    }
    aOrigin.x = aInnerMargin.left;
    if (NS_AUTOMARGIN == aInnerMargin.bottom) {
      aInnerMargin.bottom = 0;
    }
    if (NS_AUTOMARGIN == aInnerMargin.top) {
      nsCollapsingMargin marg;
      marg.Include(aInnerMargin.bottom);
      marg.Include(aCaptionMargin.top);
      nscoord collapseMargin = marg.get();
      nscoord height = aInnerSize.height + collapseMargin + aCaptionSize.height;
      aInnerMargin.top = CalcAutoMargin(aInnerMargin.top, aCaptionMargin.bottom,
                                        aContainBlockSize.height, height);
    }
    aOrigin.y = aInnerMargin.top;
  } break;
  case NS_STYLE_CAPTION_SIDE_LEFT: {
    
    if (NS_AUTOMARGIN == aInnerMargin.left) {
      aInnerMargin.left = CalcAutoMargin(aInnerMargin.left, aInnerMargin.right,
                                         aContainBlockSize.width, aInnerSize.width);
      
    }
    if (aInnerMargin.left < minCapWidth) {
      // shift the inner table to get some place for the caption
      aInnerMargin.right += aInnerMargin.left - minCapWidth;
      aInnerMargin.right  = PR_MAX(0, aInnerMargin.right);
      aInnerMargin.left   = minCapWidth;
    }
    aOrigin.x = aInnerMargin.left;
    if (NS_AUTOMARGIN == aInnerMargin.top) {
      aInnerMargin.top = 0;
    }
    aOrigin.y = aInnerMargin.top;
    switch(GetCaptionVerticalAlign()) {
      case NS_STYLE_VERTICAL_ALIGN_MIDDLE:
        aOrigin.y = PR_MAX(aInnerMargin.top, (aCaptionSize.height - aInnerSize.height) / 2);
        break;
      case NS_STYLE_VERTICAL_ALIGN_BOTTOM:
        aOrigin.y = PR_MAX(aInnerMargin.top, aCaptionSize.height - aInnerSize.height);
        break;
      default:
        break;
    }
  } break;
  case NS_STYLE_CAPTION_SIDE_RIGHT: {
    if (NS_AUTOMARGIN == aInnerMargin.right) {
      aInnerMargin.right = CalcAutoMargin(aInnerMargin.left, aInnerMargin.right,
                                          aContainBlockSize.width, aInnerSize.width);
      if (aInnerMargin.right < minCapWidth) {
        // shift the inner table to get some place for the caption
        aInnerMargin.left -= aInnerMargin.right - minCapWidth;
        aInnerMargin.left  = PR_MAX(0, aInnerMargin.left);
        aInnerMargin.right = minCapWidth;
      }
    }
    aOrigin.x = aInnerMargin.left;
    if (NS_AUTOMARGIN == aInnerMargin.top) {
      aInnerMargin.top = 0;
    }
    aOrigin.y = aInnerMargin.top;
    switch(GetCaptionVerticalAlign()) {
      case NS_STYLE_VERTICAL_ALIGN_MIDDLE:
        aOrigin.y = PR_MAX(aInnerMargin.top, (aCaptionSize.height - aInnerSize.height) / 2);
        break;
      case NS_STYLE_VERTICAL_ALIGN_BOTTOM:
        aOrigin.y = PR_MAX(aInnerMargin.top, aCaptionSize.height - aInnerSize.height);
        break;
      default:
        break;
    }
  } break;
  default: { // top
    NS_ASSERTION(aCaptionSide == NS_STYLE_CAPTION_SIDE_TOP ||
                 aCaptionSide == NS_STYLE_CAPTION_SIDE_TOP_OUTSIDE ||
                 aCaptionSide == NO_SIDE,
                 "unexpected caption side");
    if (NS_AUTOMARGIN == aInnerMargin.left) {
      aInnerMargin.left = CalcAutoMargin(aInnerMargin.left, aInnerMargin.right,
                                         aContainBlockSize.width, aInnerSize.width);
    }
    aOrigin.x = aInnerMargin.left;
    if (NS_AUTOMARGIN == aInnerMargin.top) {
      aInnerMargin.top = 0;
    }
    nsCollapsingMargin marg;
    marg.Include(aCaptionMargin.bottom);
    marg.Include(aInnerMargin.top);
    nscoord collapseMargin = marg.get();
    if (NS_AUTOMARGIN == aInnerMargin.bottom) {
      nscoord height = aCaptionSize.height + collapseMargin + aInnerSize.height;
      aInnerMargin.bottom = CalcAutoMargin(aCaptionMargin.bottom, aInnerMargin.top,
                                           aContainBlockSize.height, height);
    }
    aOrigin.y = aCaptionMargin.top + aCaptionSize.height + collapseMargin;
  } break;
  }
  return NS_OK;
}

// helper method for determining if this is a nested table or not
PRBool 
nsTableOuterFrame::IsNested(const nsHTMLReflowState& aReflowState) const
{
  // Walk up the reflow state chain until we find a cell or the root
  const nsHTMLReflowState* rs = aReflowState.parentReflowState;
  while (rs) {
    if (nsGkAtoms::tableFrame == rs->frame->GetType()) {
      return PR_TRUE;
    }
    rs = rs->parentReflowState;
  }
  return PR_FALSE;
}

void
nsTableOuterFrame::OuterBeginReflowChild(nsPresContext*           aPresContext,
                                         nsIFrame*                aChildFrame,
                                         const nsHTMLReflowState& aOuterRS,
                                         void*                    aChildRSSpace,
                                         nscoord                  aAvailWidth)
{ 
  // work around pixel rounding errors, round down to ensure we don't exceed the avail height in
  nscoord availHeight = aOuterRS.availableHeight;
  if (NS_UNCONSTRAINEDSIZE != availHeight) {
    if (mCaptionFrame == aChildFrame) {
      availHeight = NS_UNCONSTRAINEDSIZE;
    } else {
      nsMargin margin;
      GetMargin(aPresContext, aOuterRS, aChildFrame, aOuterRS.availableWidth,
                margin);
    
      NS_ASSERTION(NS_UNCONSTRAINEDSIZE != margin.top, "No unconstrainedsize arithmetic, please");
      availHeight -= margin.top;
 
      NS_ASSERTION(NS_UNCONSTRAINEDSIZE != margin.bottom, "No unconstrainedsize arithmetic, please");
      availHeight -= margin.bottom;
    }
  }
  nsSize availSize(aAvailWidth, availHeight);
  // create and init the child reflow state, using placement new on
  // stack space allocated by the caller, so that the caller can destroy
  // it
  nsHTMLReflowState &childRS = * new (aChildRSSpace)
    nsHTMLReflowState(aPresContext, aOuterRS, aChildFrame, availSize,
                      -1, -1, PR_FALSE);
  InitChildReflowState(*aPresContext, childRS);

  // see if we need to reset top of page due to a caption
  if (mCaptionFrame) {
    PRUint8 captionSide = GetCaptionSide();
    if (((captionSide == NS_STYLE_CAPTION_SIDE_BOTTOM ||
          captionSide == NS_STYLE_CAPTION_SIDE_BOTTOM_OUTSIDE) &&
         mCaptionFrame == aChildFrame) || 
        ((captionSide == NS_STYLE_CAPTION_SIDE_TOP ||
          captionSide == NS_STYLE_CAPTION_SIDE_TOP_OUTSIDE) &&
         mInnerTableFrame == aChildFrame)) {
      childRS.mFlags.mIsTopOfPage = PR_FALSE;
    }
  }
}

nsresult
nsTableOuterFrame::OuterDoReflowChild(nsPresContext*             aPresContext,
                                      nsIFrame*                  aChildFrame,
                                      const nsHTMLReflowState&   aChildRS,
                                      nsHTMLReflowMetrics&       aMetrics,
                                      nsReflowStatus&            aStatus)
{ 

  // use the current position as a best guess for placement
  nsPoint childPt = aChildFrame->GetPosition();
  return ReflowChild(aChildFrame, aPresContext, aMetrics, aChildRS,
                     childPt.x, childPt.y, NS_FRAME_NO_MOVE_FRAME, aStatus);
}

void 
nsTableOuterFrame::UpdateReflowMetrics(PRUint8              aCaptionSide,
                                       nsHTMLReflowMetrics& aMet,
                                       const nsMargin&      aInnerMargin,
                                       const nsMargin&      aCaptionMargin)
{
  SetDesiredSize(aCaptionSide, aInnerMargin, aCaptionMargin,
                 aMet.width, aMet.height);

  aMet.mOverflowArea = nsRect(0, 0, aMet.width, aMet.height);
  ConsiderChildOverflow(aMet.mOverflowArea, mInnerTableFrame);
  if (mCaptionFrame) {
    ConsiderChildOverflow(aMet.mOverflowArea, mCaptionFrame);
  }
  FinishAndStoreOverflow(&aMet);
}

NS_METHOD nsTableOuterFrame::Reflow(nsPresContext*           aPresContext,
                                    nsHTMLReflowMetrics&     aDesiredSize,
                                    const nsHTMLReflowState& aOuterRS,
                                    nsReflowStatus&          aStatus)
{
  DO_GLOBAL_REFLOW_COUNT("nsTableOuterFrame");
  DISPLAY_REFLOW(aPresContext, this, aOuterRS, aDesiredSize, aStatus);

  // We desperately need an inner table frame,
  // if this fails fix the frame constructor
  if (mFrames.IsEmpty() || !mInnerTableFrame) {
    NS_ERROR("incomplete children");
    return NS_ERROR_FAILURE;
  }
  nsresult rv = NS_OK;
  PRUint8 captionSide = GetCaptionSide();

  // Initialize out parameters
  aDesiredSize.width = aDesiredSize.height = 0;
  aStatus = NS_FRAME_COMPLETE;

  if (!(GetStateBits() & NS_FRAME_FIRST_REFLOW)) {
    // Set up our kids.  They're already present, on an overflow list, 
    // or there are none so we'll create them now
    MoveOverflowToChildList(aPresContext);
  }

  // Use longs to get more-aligned space.
  #define LONGS_IN_HTMLRS \
    ((sizeof(nsHTMLReflowState) + sizeof(long) - 1) / sizeof(long))
  long captionRSSpace[LONGS_IN_HTMLRS];
  nsHTMLReflowState *captionRS =
    static_cast<nsHTMLReflowState*>((void*)captionRSSpace);
  long innerRSSpace[LONGS_IN_HTMLRS];
  nsHTMLReflowState *innerRS =
    static_cast<nsHTMLReflowState*>((void*) innerRSSpace);

  // ComputeAutoSize has to match this logic.
  if (captionSide == NO_SIDE) {
    // We don't have a caption.
    OuterBeginReflowChild(aPresContext, mInnerTableFrame, aOuterRS,
                          innerRSSpace, aOuterRS.ComputedWidth());
  } else if (captionSide == NS_STYLE_CAPTION_SIDE_LEFT ||
             captionSide == NS_STYLE_CAPTION_SIDE_RIGHT) {
    // nsTableCaptionFrame::ComputeAutoSize takes care of making side
    // captions small.  Compute the caption's size first, and tell the
    // table to fit in what's left.
    OuterBeginReflowChild(aPresContext, mCaptionFrame, aOuterRS,
                          captionRSSpace, aOuterRS.ComputedWidth());
    nscoord innerAvailWidth = aOuterRS.ComputedWidth() -
      (captionRS->ComputedWidth() + captionRS->mComputedMargin.LeftRight() +
       captionRS->mComputedBorderPadding.LeftRight());
    OuterBeginReflowChild(aPresContext, mInnerTableFrame, aOuterRS,
                          innerRSSpace, innerAvailWidth);

  } else if (captionSide == NS_STYLE_CAPTION_SIDE_TOP ||
             captionSide == NS_STYLE_CAPTION_SIDE_BOTTOM) {
    // Compute the table's size first, and then prevent the caption from
    // being wider unless it has to be.
    //
    // Note that CSS 2.1 (but not 2.0) says:
    //   The width of the anonymous box is the border-edge width of the
    //   table box inside it
    // We don't actually make our anonymous box that width (if we did,
    // it would break 'auto' margins), but this effectively does that.
    OuterBeginReflowChild(aPresContext, mInnerTableFrame, aOuterRS,
                          innerRSSpace, aOuterRS.ComputedWidth());
    // It's good that CSS 2.1 says not to include margins, since we
    // can't, since they already been converted so they exactly
    // fill the available width (ignoring the margin on one side if
    // neither are auto).  (We take advantage of that later when we call
    // GetCaptionOrigin, though.)
    nscoord innerBorderWidth = innerRS->ComputedWidth() +
                               innerRS->mComputedBorderPadding.LeftRight();
    OuterBeginReflowChild(aPresContext, mCaptionFrame, aOuterRS,
                          captionRSSpace, innerBorderWidth);
  } else {
    NS_ASSERTION(captionSide == NS_STYLE_CAPTION_SIDE_TOP_OUTSIDE ||
                 captionSide == NS_STYLE_CAPTION_SIDE_BOTTOM_OUTSIDE,
                 "unexpected caption-side");
    // Size the table and the caption independently.
    OuterBeginReflowChild(aPresContext, mCaptionFrame, aOuterRS,
                          captionRSSpace, aOuterRS.ComputedWidth());
    OuterBeginReflowChild(aPresContext, mInnerTableFrame, aOuterRS,
                          innerRSSpace, aOuterRS.ComputedWidth());
  }

  // First reflow the caption.
  nsHTMLReflowMetrics captionMet;
  nsSize captionSize;
  nsMargin captionMargin;
  if (mCaptionFrame) {
    nsReflowStatus capStatus; // don't let the caption cause incomplete
    rv = OuterDoReflowChild(aPresContext, mCaptionFrame, *captionRS,
                            captionMet, capStatus);
    if (NS_FAILED(rv)) return rv;
    captionSize.width = captionMet.width;
    captionSize.height = captionMet.height;
    captionMargin = captionRS->mComputedMargin;
  } else {
    captionSize.SizeTo(0,0);
    captionMargin.SizeTo(0,0,0,0);
  }

  // Then, now that we know how much to reduce the width of the inner
  // table to account for side captions, reflow the inner table.
  nsHTMLReflowMetrics innerMet;
  rv = OuterDoReflowChild(aPresContext, mInnerTableFrame, *innerRS,
                          innerMet, aStatus);
  if (NS_FAILED(rv)) return rv;
  nsSize innerSize;
  innerSize.width = innerMet.width;
  innerSize.height = innerMet.height;
  nsMargin innerMargin = innerRS->mComputedMargin;

  nsSize   containSize = GetContainingBlockSize(aOuterRS);

  // Now that we've reflowed both we can place them.
  // XXXldb Most of the input variables here are now uninitialized!

  // XXX Need to recompute inner table's auto margins for the case of side
  // captions.  (Caption's are broken too, but that should be fixed earlier.)

  if (mCaptionFrame) {
    nsPoint captionOrigin;
    GetCaptionOrigin(captionSide, containSize, innerSize, 
                     innerMargin, captionSize, captionMargin, captionOrigin);
    FinishReflowChild(mCaptionFrame, aPresContext, captionRS, captionMet,
                      captionOrigin.x, captionOrigin.y, 0);
    captionRS->~nsHTMLReflowState();
  }
  // XXX If the height is constrained then we need to check whether
  // everything still fits...

  nsPoint innerOrigin;
  GetInnerOrigin(captionSide, containSize, captionSize, 
                 captionMargin, innerSize, innerMargin, innerOrigin);
  FinishReflowChild(mInnerTableFrame, aPresContext, innerRS, innerMet,
                    innerOrigin.x, innerOrigin.y, 0);
  innerRS->~nsHTMLReflowState();

  UpdateReflowMetrics(captionSide, aDesiredSize, innerMargin, captionMargin);
  
  // Return our desired rect

  NS_FRAME_SET_TRUNCATION(aStatus, aOuterRS, aDesiredSize);
  return rv;
}

#ifdef NS_DEBUG
NS_METHOD nsTableOuterFrame::VerifyTree() const
{
  return NS_OK;
}
#endif

nsIAtom*
nsTableOuterFrame::GetType() const
{
  return nsGkAtoms::tableOuterFrame;
}

/* ----- global methods ----- */

/*------------------ nsITableLayout methods ------------------------------*/
NS_IMETHODIMP 
nsTableOuterFrame::GetCellDataAt(PRInt32 aRowIndex, PRInt32 aColIndex, 
                                 nsIDOMElement* &aCell,   //out params
                                 PRInt32& aStartRowIndex, PRInt32& aStartColIndex, 
                                 PRInt32& aRowSpan, PRInt32& aColSpan,
                                 PRInt32& aActualRowSpan, PRInt32& aActualColSpan,
                                 PRBool& aIsSelected)
{
  NS_ASSERTION(mInnerTableFrame, "no inner table frame yet?");
  
  return mInnerTableFrame->GetCellDataAt(aRowIndex, aColIndex, aCell,
                                        aStartRowIndex, aStartColIndex, 
                                        aRowSpan, aColSpan, aActualRowSpan,
                                        aActualColSpan, aIsSelected);
}

NS_IMETHODIMP
nsTableOuterFrame::GetTableSize(PRInt32& aRowCount, PRInt32& aColCount)
{
  NS_ASSERTION(mInnerTableFrame, "no inner table frame yet?");

  return mInnerTableFrame->GetTableSize(aRowCount, aColCount);
}

NS_IMETHODIMP
nsTableOuterFrame::GetIndexByRowAndColumn(PRInt32 aRow, PRInt32 aColumn,
                                          PRInt32 *aIndex)
{
  NS_ENSURE_ARG_POINTER(aIndex);

  NS_ASSERTION(mInnerTableFrame, "no inner table frame yet?");
  return mInnerTableFrame->GetIndexByRowAndColumn(aRow, aColumn, aIndex);
}

NS_IMETHODIMP
nsTableOuterFrame::GetRowAndColumnByIndex(PRInt32 aIndex,
                                          PRInt32 *aRow, PRInt32 *aColumn)
{
  NS_ENSURE_ARG_POINTER(aRow);
  NS_ENSURE_ARG_POINTER(aColumn);

  NS_ASSERTION(mInnerTableFrame, "no inner table frame yet?");
  return mInnerTableFrame->GetRowAndColumnByIndex(aIndex, aRow, aColumn);
}

/*---------------- end of nsITableLayout implementation ------------------*/


nsIFrame*
NS_NewTableOuterFrame(nsIPresShell* aPresShell, nsStyleContext* aContext)
{
  return new (aPresShell) nsTableOuterFrame(aContext);
}

#ifdef DEBUG
NS_IMETHODIMP
nsTableOuterFrame::GetFrameName(nsAString& aResult) const
{
  return MakeFrameName(NS_LITERAL_STRING("TableOuter"), aResult);
}
#endif

