/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=2 sw=2 et tw=78:
 * ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is Novell code.
 *
 * The Initial Developer of the Original Code is Novell Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *     robert@ocallahan.org
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
 * ***** END LICENSE BLOCK *****
 */

/*
 * structures that represent things to be painted (ordered in z-order),
 * used during painting and hit testing
 */

#include "nsDisplayList.h"

#include "nsCSSRendering.h"
#include "nsISelectionController.h"
#include "nsIPresShell.h"
#include "nsRegion.h"
#include "nsFrameManager.h"
#include "gfxContext.h"

nsDisplayListBuilder::nsDisplayListBuilder(nsIFrame* aReferenceFrame,
    PRBool aIsForEvents, PRBool aBuildCaret, nsIFrame* aMovingFrame)
    : mReferenceFrame(aReferenceFrame),
      mMovingFrame(aMovingFrame),
      mIgnoreScrollFrame(nsnull),
      mBuildCaret(aBuildCaret),
      mEventDelivery(aIsForEvents),
      mIsAtRootOfPseudoStackingContext(PR_FALSE),
      mPaintAllFrames(PR_FALSE) {
  PL_InitArenaPool(&mPool, "displayListArena", 1024, sizeof(void*)-1);

  nsPresContext* pc = aReferenceFrame->PresContext();
  nsIPresShell *shell = pc->PresShell();
  PRBool suppressed;
  shell->IsPaintingSuppressed(&suppressed);
  mIsBackgroundOnly = suppressed;
  if (pc->IsRenderingOnlySelection()) {
    nsCOMPtr<nsISelectionController> selcon(do_QueryInterface(shell));
    if (selcon) {
      selcon->GetSelection(nsISelectionController::SELECTION_NORMAL,
                           getter_AddRefs(mBoundingSelection));
    }
  }

  if (mIsBackgroundOnly) {
    mBuildCaret = PR_FALSE;
  }
}

// Destructor function for the dirty rect property
static void
DestroyRectFunc(void*    aFrame,
                nsIAtom* aPropertyName,
                void*    aPropertyValue,
                void*    aDtorData)
{
  delete static_cast<nsRect*>(aPropertyValue);
}

static void MarkFrameForDisplay(nsIFrame* aFrame, nsIFrame* aStopAtFrame) {
  nsFrameManager* frameManager = aFrame->PresContext()->PresShell()->FrameManager();

  for (nsIFrame* f = aFrame; f;
       f = nsLayoutUtils::GetParentOrPlaceholderFor(frameManager, f)) {
    if (f->GetStateBits() & NS_FRAME_FORCE_DISPLAY_LIST_DESCEND_INTO)
      return;
    f->AddStateBits(NS_FRAME_FORCE_DISPLAY_LIST_DESCEND_INTO);
    if (f == aStopAtFrame) {
      // we've reached a frame that we know will be painted, so we can stop.
      break;
    }
  }
}

static void MarkOutOfFlowFrameForDisplay(nsIFrame* aDirtyFrame, nsIFrame* aFrame,
                                         const nsRect& aDirtyRect) {
  nsRect dirty = aDirtyRect - aFrame->GetOffsetTo(aDirtyFrame);
  nsRect overflowRect = aFrame->GetOverflowRect();
  if (!dirty.IntersectRect(dirty, overflowRect))
    return;
  // if "new nsRect" fails, this won't do anything, but that's okay
  aFrame->SetProperty(nsGkAtoms::outOfFlowDirtyRectProperty,
                      new nsRect(dirty), DestroyRectFunc);

  MarkFrameForDisplay(aFrame, aDirtyFrame);
}

static void UnmarkFrameForDisplay(nsIFrame* aFrame) {
  aFrame->DeleteProperty(nsGkAtoms::outOfFlowDirtyRectProperty);

  nsFrameManager* frameManager = aFrame->PresContext()->PresShell()->FrameManager();

  for (nsIFrame* f = aFrame; f;
       f = nsLayoutUtils::GetParentOrPlaceholderFor(frameManager, f)) {
    if (!(f->GetStateBits() & NS_FRAME_FORCE_DISPLAY_LIST_DESCEND_INTO))
      return;
    f->RemoveStateBits(NS_FRAME_FORCE_DISPLAY_LIST_DESCEND_INTO);
  }
}

nsDisplayListBuilder::~nsDisplayListBuilder() {
  NS_ASSERTION(mFramesMarkedForDisplay.Length() == 0,
               "All frames should have been unmarked");
  NS_ASSERTION(mPresShellStates.Length() == 0,
               "All presshells should have been exited");

  PL_FreeArenaPool(&mPool);
  PL_FinishArenaPool(&mPool);
}

nsICaret *
nsDisplayListBuilder::GetCaret() {
  nsCOMPtr<nsICaret> caret;
  CurrentPresShellState()->mPresShell->GetCaret(getter_AddRefs(caret));
  return caret;
}

void
nsDisplayListBuilder::EnterPresShell(nsIFrame* aReferenceFrame,
                                     const nsRect& aDirtyRect) {
  PresShellState* state = mPresShellStates.AppendElement();
  if (!state)
    return;
  state->mPresShell = aReferenceFrame->PresContext()->PresShell();
  state->mCaretFrame = nsnull;
  state->mFirstFrameMarkedForDisplay = mFramesMarkedForDisplay.Length();

  if (!mBuildCaret)
    return;

  nsCOMPtr<nsICaret> caret;
  state->mPresShell->GetCaret(getter_AddRefs(caret));
  state->mCaretFrame = caret->GetCaretFrame();

  if (state->mCaretFrame) {
    // Check if the dirty rect intersects with the caret's dirty rect.
    nsRect caretRect =
      caret->GetCaretRect() + state->mCaretFrame->GetOffsetTo(aReferenceFrame);
    if (caretRect.Intersects(aDirtyRect)) {
      // Okay, our rects intersect, let's mark the frame and all of its ancestors.
      mFramesMarkedForDisplay.AppendElement(state->mCaretFrame);
      MarkFrameForDisplay(state->mCaretFrame, nsnull);
    }
  }
}

void
nsDisplayListBuilder::LeavePresShell(nsIFrame* aReferenceFrame,
                                     const nsRect& aDirtyRect)
{
  if (CurrentPresShellState()->mPresShell != aReferenceFrame->PresContext()->PresShell()) {
    // Must have not allocated a state for this presshell, presumably due
    // to OOM.
    return;
  }

  // Unmark and pop off the frames marked for display in this pres shell.
  PRUint32 firstFrameForShell = CurrentPresShellState()->mFirstFrameMarkedForDisplay;
  for (PRUint32 i = firstFrameForShell;
       i < mFramesMarkedForDisplay.Length(); ++i) {
    UnmarkFrameForDisplay(mFramesMarkedForDisplay[i]);
  }
  mFramesMarkedForDisplay.SetLength(firstFrameForShell);
  mPresShellStates.SetLength(mPresShellStates.Length() - 1);
}

void
nsDisplayListBuilder::MarkFramesForDisplayList(nsIFrame* aDirtyFrame, nsIFrame* aFrames,
                                               const nsRect& aDirtyRect) {
  while (aFrames) {
    mFramesMarkedForDisplay.AppendElement(aFrames);
    MarkOutOfFlowFrameForDisplay(aDirtyFrame, aFrames, aDirtyRect);
    aFrames = aFrames->GetNextSibling();
  }
}

void*
nsDisplayListBuilder::Allocate(size_t aSize) {
  void *tmp;
  PL_ARENA_ALLOCATE(tmp, &mPool, aSize);
  return tmp;
}

void nsDisplayListSet::MoveTo(const nsDisplayListSet& aDestination) const
{
  aDestination.BorderBackground()->AppendToTop(BorderBackground());
  aDestination.BlockBorderBackgrounds()->AppendToTop(BlockBorderBackgrounds());
  aDestination.Floats()->AppendToTop(Floats());
  aDestination.Content()->AppendToTop(Content());
  aDestination.PositionedDescendants()->AppendToTop(PositionedDescendants());
  aDestination.Outlines()->AppendToTop(Outlines());
}

// Suitable for leaf items only, overridden by nsDisplayWrapList
PRBool
nsDisplayItem::OptimizeVisibility(nsDisplayListBuilder* aBuilder,
                                  nsRegion* aVisibleRegion) {
  nsRect bounds = GetBounds(aBuilder);
  if (!aVisibleRegion->Intersects(bounds))
    return PR_FALSE;
  
  nsIFrame* f = GetUnderlyingFrame();
  NS_ASSERTION(f, "GetUnderlyingFrame() must return non-null for leaf items");
  if (aBuilder->HasMovingFrames() && aBuilder->IsMovingFrame(f)) {
    // If this frame is in the moving subtree, and it doesn't
    // require repainting just because it's moved, then just remove it now
    // because it's not relevant.
    if (!IsVaryingRelativeToFrame(aBuilder, aBuilder->GetRootMovingFrame()))
      return PR_FALSE;
    // keep it, but don't let it cover other display items (see nsLayoutUtils::
    // ComputeRepaintRegionForCopy)
    return PR_TRUE;
  }

  if (IsOpaque(aBuilder)) {
    aVisibleRegion->SimpleSubtract(bounds);
  }
  return PR_TRUE;
}

void
nsDisplayList::FlattenTo(nsTArray<nsDisplayItem*>* aElements) {
  nsDisplayItem* item;
  while ((item = RemoveBottom()) != nsnull) {
    if (item->GetType() == nsDisplayItem::TYPE_WRAPLIST) {
      item->GetList()->FlattenTo(aElements);
      item->~nsDisplayItem();
    } else {
      aElements->AppendElement(item);
    }
  }
}

void
nsDisplayList::OptimizeVisibility(nsDisplayListBuilder* aBuilder,
                                  nsRegion* aVisibleRegion) {
  nsAutoTArray<nsDisplayItem*, 512> elements;
  FlattenTo(&elements);

  for (PRInt32 i = elements.Length() - 1; i >= 0; --i) {
    nsDisplayItem* item = elements[i];
    nsDisplayItem* belowItem = i < 1 ? nsnull : elements[i - 1];

    if (belowItem && item->TryMerge(aBuilder, belowItem)) {
      belowItem->~nsDisplayItem();
      elements.ReplaceElementsAt(i - 1, 1, item);
      continue;
    }
    
    if (item->OptimizeVisibility(aBuilder, aVisibleRegion)) {
      AppendToBottom(item);
    } else {
      item->~nsDisplayItem();
    }
  }
}

void nsDisplayList::Paint(nsDisplayListBuilder* aBuilder, nsIRenderingContext* aCtx,
                          const nsRect& aDirtyRect) const {
  for (nsDisplayItem* i = GetBottom(); i != nsnull; i = i->GetAbove()) {
    i->Paint(aBuilder, aCtx, aDirtyRect);
  }
  nsCSSRendering::DidPaint();
}

PRUint32 nsDisplayList::Count() const {
  PRUint32 count = 0;
  for (nsDisplayItem* i = GetBottom(); i; i = i->GetAbove()) {
    ++count;
  }
  return count;
}

nsDisplayItem* nsDisplayList::RemoveBottom() {
  nsDisplayItem* item = mSentinel.mAbove;
  if (!item)
    return nsnull;
  mSentinel.mAbove = item->mAbove;
  if (item == mTop) {
    // must have been the only item
    mTop = &mSentinel;
  }
  item->mAbove = nsnull;
  return item;
}

void nsDisplayList::DeleteBottom() {
  nsDisplayItem* item = RemoveBottom();
  if (item) {
    item->~nsDisplayItem();
  }
}

void nsDisplayList::DeleteAll() {
  nsDisplayItem* item;
  while ((item = RemoveBottom()) != nsnull) {
    item->~nsDisplayItem();
  }
}

nsIFrame* nsDisplayList::HitTest(nsDisplayListBuilder* aBuilder, nsPoint aPt,
                                 nsDisplayItem::HitTestState* aState) const {
  PRInt32 itemBufferStart = aState->mItemBuffer.Length();
  nsDisplayItem* item;
  for (item = GetBottom(); item; item = item->GetAbove()) {
    aState->mItemBuffer.AppendElement(item);
  }
  for (PRInt32 i = aState->mItemBuffer.Length() - 1; i >= itemBufferStart; --i) {
    // Pop element off the end of the buffer. We want to shorten the buffer
    // so that recursive calls to HitTest have more buffer space.
    item = aState->mItemBuffer[i];
    aState->mItemBuffer.SetLength(i);

    if (item->GetBounds(aBuilder).Contains(aPt)) {
      nsIFrame* f = item->HitTest(aBuilder, aPt, aState);
      // Handle the XUL 'mousethrough' feature.
      if (f) {
        if (!f->GetMouseThrough()) {
          aState->mItemBuffer.SetLength(itemBufferStart);
          return f;
        }
      }
    }
  }
  NS_ASSERTION(aState->mItemBuffer.Length() == itemBufferStart,
               "How did we forget to pop some elements?");
  return nsnull;
}

static void Sort(nsDisplayList* aList, PRInt32 aCount, nsDisplayList::SortLEQ aCmp,
                 void* aClosure) {
  if (aCount < 2)
    return;

  nsDisplayList list1;
  nsDisplayList list2;
  int i;
  PRInt32 half = aCount/2;
  PRBool sorted = PR_TRUE;
  nsDisplayItem* prev = nsnull;
  for (i = 0; i < aCount; ++i) {
    nsDisplayItem* item = aList->RemoveBottom();
    (i < half ? &list1 : &list2)->AppendToTop(item);
    if (sorted && prev && !aCmp(prev, item, aClosure)) {
      sorted = PR_FALSE;
    }
    prev = item;
  }
  if (sorted) {
    aList->AppendToTop(&list1);
    aList->AppendToTop(&list2);
    return;
  }
  
  Sort(&list1, half, aCmp, aClosure);
  Sort(&list2, aCount - half, aCmp, aClosure);

  for (i = 0; i < aCount; ++i) {
    if (list1.GetBottom() &&
        (!list2.GetBottom() ||
         aCmp(list1.GetBottom(), list2.GetBottom(), aClosure))) {
      aList->AppendToTop(list1.RemoveBottom());
    } else {
      aList->AppendToTop(list2.RemoveBottom());
    }
  }
}

static PRBool IsContentLEQ(nsDisplayItem* aItem1, nsDisplayItem* aItem2,
                           void* aClosure) {
  // These GetUnderlyingFrame calls return non-null because we're only used
  // in sorting
  return nsLayoutUtils::CompareTreePosition(
      aItem1->GetUnderlyingFrame()->GetContent(),
      aItem2->GetUnderlyingFrame()->GetContent(),
      static_cast<nsIContent*>(aClosure)) <= 0;
}

static PRBool IsZOrderLEQ(nsDisplayItem* aItem1, nsDisplayItem* aItem2,
                          void* aClosure) {
  // These GetUnderlyingFrame calls return non-null because we're only used
  // in sorting
  PRInt32 diff = nsLayoutUtils::GetZIndex(aItem1->GetUnderlyingFrame()) -
    nsLayoutUtils::GetZIndex(aItem2->GetUnderlyingFrame());
  if (diff == 0)
    return IsContentLEQ(aItem1, aItem2, aClosure);
  return diff < 0;
}

void nsDisplayList::ExplodeAnonymousChildLists(nsDisplayListBuilder* aBuilder) {
  // See if there's anything to do
  PRBool anyAnonymousItems = PR_FALSE;
  nsDisplayItem* i;
  for (i = GetBottom(); i != nsnull; i = i->GetAbove()) {
    if (!i->GetUnderlyingFrame()) {
      anyAnonymousItems = PR_TRUE;
      break;
    }
  }
  if (!anyAnonymousItems)
    return;

  nsDisplayList tmp;
  while ((i = RemoveBottom()) != nsnull) {
    if (i->GetUnderlyingFrame()) {
      tmp.AppendToTop(i);
    } else {
      nsDisplayList* list = i->GetList();
      NS_ASSERTION(list, "leaf items can't be anonymous");
      list->ExplodeAnonymousChildLists(aBuilder);
      nsDisplayItem* j;
      while ((j = list->RemoveBottom()) != nsnull) {
        tmp.AppendToTop(static_cast<nsDisplayWrapList*>(i)->
            WrapWithClone(aBuilder, j));
      }
      i->~nsDisplayItem();
    }
  }
  
  AppendToTop(&tmp);
}

void nsDisplayList::SortByZOrder(nsDisplayListBuilder* aBuilder,
                                 nsIContent* aCommonAncestor) {
  Sort(aBuilder, IsZOrderLEQ, aCommonAncestor);
}

void nsDisplayList::SortByContentOrder(nsDisplayListBuilder* aBuilder,
                                       nsIContent* aCommonAncestor) {
  Sort(aBuilder, IsContentLEQ, aCommonAncestor);
}

void nsDisplayList::Sort(nsDisplayListBuilder* aBuilder,
                         SortLEQ aCmp, void* aClosure) {
  ExplodeAnonymousChildLists(aBuilder);
  ::Sort(this, Count(), aCmp, aClosure);
}

PRBool
nsDisplayBackground::IsOpaque(nsDisplayListBuilder* aBuilder) {
  // theme background overrides any other background
  if (mIsThemed)
    return PR_FALSE;

  PRBool isCanvas;
  const nsStyleBackground* bg;
  PRBool hasBG =
    nsCSSRendering::FindBackground(mFrame->PresContext(), mFrame, &bg, &isCanvas);
  if (!hasBG || (bg->mBackgroundFlags & NS_STYLE_BG_COLOR_TRANSPARENT) ||
      bg->mBackgroundClip != NS_STYLE_BG_CLIP_BORDER ||
      nsLayoutUtils::HasNonZeroSide(mFrame->GetStyleBorder()->mBorderRadius) ||
      NS_GET_A(bg->mBackgroundColor) < 255)
    return PR_FALSE;
  return PR_TRUE;
}

PRBool
nsDisplayBackground::IsUniform(nsDisplayListBuilder* aBuilder) {
  // theme background overrides any other background
  if (mIsThemed)
    return PR_FALSE;

  PRBool isCanvas;
  const nsStyleBackground* bg;
  PRBool hasBG =
    nsCSSRendering::FindBackground(mFrame->PresContext(), mFrame, &bg, &isCanvas);
  if (!hasBG)
    return PR_TRUE;
  if ((bg->mBackgroundFlags & NS_STYLE_BG_IMAGE_NONE) &&
      !nsLayoutUtils::HasNonZeroSide(mFrame->GetStyleBorder()->mBorderRadius) &&
      bg->mBackgroundClip == NS_STYLE_BG_CLIP_BORDER)
    return PR_TRUE;
  return PR_FALSE;
}

PRBool
nsDisplayBackground::IsVaryingRelativeToFrame(nsDisplayListBuilder* aBuilder,
    nsIFrame* aAncestorFrame)
{
  PRBool isCanvas;
  const nsStyleBackground* bg;
  PRBool hasBG =
    nsCSSRendering::FindBackground(mFrame->PresContext(), mFrame, &bg, &isCanvas);
  if (!hasBG)
    return PR_FALSE;
  if (!bg->HasFixedBackground())
    return PR_FALSE;

  // aAncestorFrame is the frame that is going to be moved.
  // Check if mFrame is equal to aAncestorFrame or aAncestorFrame is an
  // ancestor of mFrame in the same document. If this is true, mFrame
  // will move relative to its viewport, which means this display item will
  // change when it is moved.  If they are in different documents, we do not
  // want to return true because mFrame won't move relative to its viewport.
  for (nsIFrame* f = mFrame; f; f = f->GetParent()) {
    if (f == aAncestorFrame)
      return PR_TRUE;
  }
  return PR_FALSE;
}

void
nsDisplayBackground::Paint(nsDisplayListBuilder* aBuilder,
     nsIRenderingContext* aCtx, const nsRect& aDirtyRect) {
  nsPoint offset = aBuilder->ToReferenceFrame(mFrame);
  nsCSSRendering::PaintBackground(mFrame->PresContext(), *aCtx, mFrame,
                                  aDirtyRect, nsRect(offset, mFrame->GetSize()),
                                  *mFrame->GetStyleBorder(),
                                  *mFrame->GetStylePadding(),
                                  mFrame->HonorPrintBackgroundSettings());
}

nsRect
nsDisplayBackground::GetBounds(nsDisplayListBuilder* aBuilder) {
  if (mIsThemed)
    return mFrame->GetOverflowRect() + aBuilder->ToReferenceFrame(mFrame);

  return nsRect(aBuilder->ToReferenceFrame(mFrame), mFrame->GetSize());
}

nsRect
nsDisplayOutline::GetBounds(nsDisplayListBuilder* aBuilder) {
  return mFrame->GetOverflowRect() + aBuilder->ToReferenceFrame(mFrame);
}

void
nsDisplayOutline::Paint(nsDisplayListBuilder* aBuilder,
     nsIRenderingContext* aCtx, const nsRect& aDirtyRect) {
  // TODO join outlines together
  nsPoint offset = aBuilder->ToReferenceFrame(mFrame);
  nsCSSRendering::PaintOutline(mFrame->PresContext(), *aCtx, mFrame,
                               aDirtyRect, nsRect(offset, mFrame->GetSize()),
                               *mFrame->GetStyleBorder(),
                               *mFrame->GetStyleOutline(),                              
                               mFrame->GetStyleContext(), 0);
}

PRBool
nsDisplayOutline::OptimizeVisibility(nsDisplayListBuilder* aBuilder,
                                     nsRegion* aVisibleRegion) {
  if (!nsDisplayItem::OptimizeVisibility(aBuilder, aVisibleRegion))
    return PR_FALSE;

  const nsStyleOutline* outline = mFrame->GetStyleOutline();
  nsPoint origin = aBuilder->ToReferenceFrame(mFrame);
  if (nsRect(origin, mFrame->GetSize()).Contains(aVisibleRegion->GetBounds()) &&
      !nsLayoutUtils::HasNonZeroSide(outline->mOutlineRadius)) {
    nscoord outlineOffset;
    outline->GetOutlineOffset(outlineOffset);
    if (outlineOffset >= 0) {
      // the visible region is entirely inside the border-rect, and the outline
      // isn't rendered inside the border-rect, so the outline is not visible
      return PR_FALSE;
    }
  }

  return PR_TRUE;
}

void
nsDisplayCaret::Paint(nsDisplayListBuilder* aBuilder,
    nsIRenderingContext* aCtx, const nsRect& aDirtyRect) {
  // Note: Because we exist, we know that the caret is visible, so we don't
  // need to check for the caret's visibility.
  mCaret->PaintCaret(aBuilder, aCtx, aBuilder->ToReferenceFrame(mFrame),
                     mFrame->GetStyleColor()->mColor);
}

PRBool
nsDisplayBorder::OptimizeVisibility(nsDisplayListBuilder* aBuilder,
                                    nsRegion* aVisibleRegion) {
  if (!nsDisplayItem::OptimizeVisibility(aBuilder, aVisibleRegion))
    return PR_FALSE;

  nsRect paddingRect = mFrame->GetPaddingRect() - mFrame->GetPosition() +
    aBuilder->ToReferenceFrame(mFrame);
  if (paddingRect.Contains(aVisibleRegion->GetBounds()) &&
      !nsLayoutUtils::HasNonZeroSide(mFrame->GetStyleBorder()->mBorderRadius)) {
    // the visible region is entirely inside the content rect, and no part
    // of the border is rendered inside the content rect, so we are not
    // visible
    return PR_FALSE;
  }

  return PR_TRUE;
}

void
nsDisplayBorder::Paint(nsDisplayListBuilder* aBuilder,
     nsIRenderingContext* aCtx, const nsRect& aDirtyRect) {
  nsPoint offset = aBuilder->ToReferenceFrame(mFrame);
  nsCSSRendering::PaintBorder(mFrame->PresContext(), *aCtx, mFrame,
                              aDirtyRect, nsRect(offset, mFrame->GetSize()),
                              *mFrame->GetStyleBorder(),
                              mFrame->GetStyleContext(), mFrame->GetSkipSides());
}

nsDisplayWrapList::nsDisplayWrapList(nsIFrame* aFrame, nsDisplayList* aList)
  : nsDisplayItem(aFrame) {
  mList.AppendToTop(aList);
}

nsDisplayWrapList::nsDisplayWrapList(nsIFrame* aFrame, nsDisplayItem* aItem)
  : nsDisplayItem(aFrame) {
  mList.AppendToTop(aItem);
}

nsDisplayWrapList::~nsDisplayWrapList() {
  mList.DeleteAll();
}

nsIFrame*
nsDisplayWrapList::HitTest(nsDisplayListBuilder* aBuilder, nsPoint aPt,
                           HitTestState* aState) {
  return mList.HitTest(aBuilder, aPt, aState);
}

nsRect
nsDisplayWrapList::GetBounds(nsDisplayListBuilder* aBuilder) {
  nsRect bounds;
  for (nsDisplayItem* i = mList.GetBottom(); i != nsnull; i = i->GetAbove()) {
    bounds.UnionRect(bounds, i->GetBounds(aBuilder));
  }
  return bounds;
}

PRBool
nsDisplayWrapList::OptimizeVisibility(nsDisplayListBuilder* aBuilder,
                                      nsRegion* aVisibleRegion) {
  mList.OptimizeVisibility(aBuilder, aVisibleRegion);
  // If none of the items are visible, they will all have been deleted
  return mList.GetTop() != nsnull;
}

PRBool
nsDisplayWrapList::IsOpaque(nsDisplayListBuilder* aBuilder) {
  // We could try to do something but let's conservatively just return PR_FALSE.
  // We reimplement OptimizeVisibility and that's what really matters
  return PR_FALSE;
}

PRBool nsDisplayWrapList::IsUniform(nsDisplayListBuilder* aBuilder) {
  // We could try to do something but let's conservatively just return PR_FALSE.
  return PR_FALSE;
}

PRBool nsDisplayWrapList::IsVaryingRelativeToFrame(nsDisplayListBuilder* aBuilder,
                                                   nsIFrame* aFrame) {
  for (nsDisplayItem* i = mList.GetBottom(); i != nsnull; i = i->GetAbove()) {
    if (i->IsVaryingRelativeToFrame(aBuilder, aFrame))
      return PR_TRUE;
  }
  return PR_FALSE;
}

void nsDisplayWrapList::Paint(nsDisplayListBuilder* aBuilder,
     nsIRenderingContext* aCtx, const nsRect& aDirtyRect) {
  mList.Paint(aBuilder, aCtx, aDirtyRect);
}

static nsresult
WrapDisplayList(nsDisplayListBuilder* aBuilder, nsIFrame* aFrame,
                nsDisplayList* aList, nsDisplayWrapper* aWrapper) {
  if (!aList->GetTop())
    return NS_OK;
  nsDisplayItem* item = aWrapper->WrapList(aBuilder, aFrame, aList);
  if (!item)
    return NS_ERROR_OUT_OF_MEMORY;
  // aList was emptied
  aList->AppendToTop(item);
  return NS_OK;
}

static nsresult
WrapEachDisplayItem(nsDisplayListBuilder* aBuilder,
                    nsDisplayList* aList, nsDisplayWrapper* aWrapper) {
  nsDisplayList newList;
  nsDisplayItem* item;
  while ((item = aList->RemoveBottom())) {
    item = aWrapper->WrapItem(aBuilder, item);
    if (!item)
      return NS_ERROR_OUT_OF_MEMORY;
    newList.AppendToTop(item);
  }
  // aList was emptied
  aList->AppendToTop(&newList);
  return NS_OK;
}

nsresult nsDisplayWrapper::WrapLists(nsDisplayListBuilder* aBuilder,
    nsIFrame* aFrame, const nsDisplayListSet& aIn, const nsDisplayListSet& aOut)
{
  nsresult rv = WrapListsInPlace(aBuilder, aFrame, aIn);
  NS_ENSURE_SUCCESS(rv, rv);

  if (&aOut == &aIn)
    return NS_OK;
  aOut.BorderBackground()->AppendToTop(aIn.BorderBackground());
  aOut.BlockBorderBackgrounds()->AppendToTop(aIn.BlockBorderBackgrounds());
  aOut.Floats()->AppendToTop(aIn.Floats());
  aOut.Content()->AppendToTop(aIn.Content());
  aOut.PositionedDescendants()->AppendToTop(aIn.PositionedDescendants());
  aOut.Outlines()->AppendToTop(aIn.Outlines());
  return NS_OK;
}

nsresult nsDisplayWrapper::WrapListsInPlace(nsDisplayListBuilder* aBuilder,
    nsIFrame* aFrame, const nsDisplayListSet& aLists)
{
  nsresult rv;
  if (WrapBorderBackground()) {
    // Our border-backgrounds are in-flow
    rv = WrapDisplayList(aBuilder, aFrame, aLists.BorderBackground(), this);
    NS_ENSURE_SUCCESS(rv, rv);
  }
  // Our block border-backgrounds are in-flow
  rv = WrapDisplayList(aBuilder, aFrame, aLists.BlockBorderBackgrounds(), this);
  NS_ENSURE_SUCCESS(rv, rv);
  // The floats are not in flow
  rv = WrapEachDisplayItem(aBuilder, aLists.Floats(), this);
  NS_ENSURE_SUCCESS(rv, rv);
  // Our child content is in flow
  rv = WrapDisplayList(aBuilder, aFrame, aLists.Content(), this);
  NS_ENSURE_SUCCESS(rv, rv);
  // The positioned descendants may not be in-flow
  rv = WrapEachDisplayItem(aBuilder, aLists.PositionedDescendants(), this);
  NS_ENSURE_SUCCESS(rv, rv);
  // The outlines may not be in-flow
  return WrapEachDisplayItem(aBuilder, aLists.Outlines(), this);
}

nsDisplayOpacity::nsDisplayOpacity(nsIFrame* aFrame, nsDisplayList* aList)
    : nsDisplayWrapList(aFrame, aList), mNeedAlpha(PR_TRUE) {
  MOZ_COUNT_CTOR(nsDisplayOpacity);
}

#ifdef NS_BUILD_REFCNT_LOGGING
nsDisplayOpacity::~nsDisplayOpacity() {
  MOZ_COUNT_DTOR(nsDisplayOpacity);
}
#endif

PRBool nsDisplayOpacity::IsOpaque(nsDisplayListBuilder* aBuilder) {
  // We are never opaque, if our opacity was < 1 then we wouldn't have
  // been created.
  return PR_FALSE;
}

void nsDisplayOpacity::Paint(nsDisplayListBuilder* aBuilder,
                             nsIRenderingContext* aCtx, const nsRect& aDirtyRect)
{
  // XXX This way of handling 'opacity' creates exponential time blowup in the
  // depth of nested translucent elements. This will be fixed when we move to
  // cairo with support for real alpha channels in surfaces, so we don't have
  // to do this white/black hack anymore.
  float opacity = mFrame->GetStyleDisplay()->mOpacity;

  nsRect bounds;
  bounds.IntersectRect(GetBounds(aBuilder), aDirtyRect);

  nsCOMPtr<nsIDeviceContext> devCtx;
  aCtx->GetDeviceContext(*getter_AddRefs(devCtx));
  float a2p = 1.0f / devCtx->AppUnitsPerDevPixel();

  nsRefPtr<gfxContext> ctx = aCtx->ThebesContext();

  ctx->Save();

  ctx->NewPath();
  ctx->Rectangle(gfxRect(bounds.x * a2p,
                         bounds.y * a2p,
                         bounds.width * a2p,
                         bounds.height * a2p),
                 PR_TRUE);
  ctx->Clip();

  if (mNeedAlpha)
    ctx->PushGroup(gfxASurface::CONTENT_COLOR_ALPHA);
  else
    ctx->PushGroup(gfxASurface::CONTENT_COLOR);

  nsDisplayWrapList::Paint(aBuilder, aCtx, bounds);

  ctx->PopGroupToSource();
  ctx->SetOperator(gfxContext::OPERATOR_OVER);
  ctx->Paint(opacity);

  ctx->Restore();
}

PRBool nsDisplayOpacity::OptimizeVisibility(nsDisplayListBuilder* aBuilder,
                                            nsRegion* aVisibleRegion) {
  // Our children are translucent so we should not allow them to subtract
  // area from aVisibleRegion. We do need to find out what is visible under
  // our children in the temporary compositing buffer, because if our children
  // paint our entire bounds opaquely then we don't need an alpha channel in
  // the temporary compositing buffer.
  nsRegion visibleUnderChildren = *aVisibleRegion;
  PRBool anyVisibleChildren =
    nsDisplayWrapList::OptimizeVisibility(aBuilder, &visibleUnderChildren);
  if (!anyVisibleChildren)
    return PR_FALSE;

  mNeedAlpha = visibleUnderChildren.Intersects(GetBounds(aBuilder));
  return PR_TRUE;
}

PRBool nsDisplayOpacity::TryMerge(nsDisplayListBuilder* aBuilder, nsDisplayItem* aItem) {
  if (aItem->GetType() != TYPE_OPACITY)
    return PR_FALSE;
  // items for the same content element should be merged into a single
  // compositing group
  // aItem->GetUnderlyingFrame() returns non-null because it's nsDisplayOpacity
  if (aItem->GetUnderlyingFrame()->GetContent() != mFrame->GetContent())
    return PR_FALSE;
  mList.AppendToBottom(&static_cast<nsDisplayOpacity*>(aItem)->mList);
  return PR_TRUE;
}

nsDisplayClip::nsDisplayClip(nsIFrame* aFrame, nsDisplayItem* aItem,
    const nsRect& aRect)
   : nsDisplayWrapList(aFrame, aItem), mClip(aRect) {
  MOZ_COUNT_CTOR(nsDisplayClip);
}

nsDisplayClip::nsDisplayClip(nsIFrame* aFrame, nsDisplayList* aList,
    const nsRect& aRect)
   : nsDisplayWrapList(aFrame, aList), mClip(aRect) {
  MOZ_COUNT_CTOR(nsDisplayClip);
}

nsRect nsDisplayClip::GetBounds(nsDisplayListBuilder* aBuilder) {
  nsRect r = nsDisplayWrapList::GetBounds(aBuilder);
  r.IntersectRect(mClip, r);
  return r;
}

#ifdef NS_BUILD_REFCNT_LOGGING
nsDisplayClip::~nsDisplayClip() {
  MOZ_COUNT_DTOR(nsDisplayClip);
}
#endif

void nsDisplayClip::Paint(nsDisplayListBuilder* aBuilder,
     nsIRenderingContext* aCtx, const nsRect& aDirtyRect) {
  nsRect dirty;
  dirty.IntersectRect(mClip, aDirtyRect);
  aCtx->PushState();
  aCtx->SetClipRect(dirty, nsClipCombine_kIntersect);
  nsDisplayWrapList::Paint(aBuilder, aCtx, dirty);
  aCtx->PopState();
}

PRBool nsDisplayClip::OptimizeVisibility(nsDisplayListBuilder* aBuilder,
                                         nsRegion* aVisibleRegion) {
  nsRegion clipped;
  clipped.And(*aVisibleRegion, mClip);
  nsRegion rNew(clipped);
  PRBool anyVisible = nsDisplayWrapList::OptimizeVisibility(aBuilder, &rNew);
  nsRegion subtracted;
  subtracted.Sub(clipped, rNew);
  aVisibleRegion->SimpleSubtract(subtracted);
  return anyVisible;
}

PRBool nsDisplayClip::TryMerge(nsDisplayListBuilder* aBuilder,
                               nsDisplayItem* aItem) {
  if (aItem->GetType() != TYPE_CLIP)
    return PR_FALSE;
  nsDisplayClip* other = static_cast<nsDisplayClip*>(aItem);
  if (other->mClip != mClip)
    return PR_FALSE;
  mList.AppendToBottom(&other->mList);
  return PR_TRUE;
}

nsDisplayWrapList* nsDisplayClip::WrapWithClone(nsDisplayListBuilder* aBuilder,
                                                nsDisplayItem* aItem) {
  return new (aBuilder) nsDisplayClip(aItem->GetUnderlyingFrame(), aItem, mClip);
}
