/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "nsLeafBoxFrame.h"
#include "nsBoxFrame.h"
#include "nsCOMPtr.h"
#include "nsIDeviceContext.h"
#include "nsIFontMetrics.h"
#include "nsHTMLAtoms.h"
#include "nsXULAtoms.h"
#include "nsPresContext.h"
#include "nsIRenderingContext.h"
#include "nsStyleContext.h"
#include "nsIContent.h"
#include "nsINameSpaceManager.h"
#include "nsBoxLayoutState.h"
#include "nsWidgetsCID.h"
#include "nsIViewManager.h"
#include "nsHTMLContainerFrame.h"
#include "nsDisplayList.h"

static NS_DEFINE_IID(kWidgetCID, NS_CHILD_CID);

//
// NS_NewLeafBoxFrame
//
// Creates a new Toolbar frame and returns it
//
nsIFrame*
NS_NewLeafBoxFrame (nsIPresShell* aPresShell, nsStyleContext* aContext)
{
  return new (aPresShell) nsLeafBoxFrame(aPresShell, aContext);
} // NS_NewLeafBoxFrame

nsLeafBoxFrame::nsLeafBoxFrame(nsIPresShell* aShell, nsStyleContext* aContext)
    : nsLeafFrame(aContext), mMouseThrough(unset)
{
    mState |= NS_FRAME_IS_BOX;
}

#ifdef DEBUG_LAYOUT
void
nsLeafBoxFrame::GetBoxName(nsAutoString& aName)
{
   GetFrameName(aName);
}
#endif


/**
 * Initialize us. This is a good time to get the alignment of the box
 */
NS_IMETHODIMP
nsLeafBoxFrame::Init(
              nsIContent*      aContent,
              nsIFrame*        aParent,
              nsIFrame*        aPrevInFlow)
{
  nsresult  rv = nsLeafFrame::Init(aContent, aParent, aPrevInFlow);

   // see if we need a widget
  if (aParent && aParent->IsBoxFrame()) {
    PRBool needsWidget = PR_FALSE;
    aParent->ChildrenMustHaveWidgets(needsWidget);
    if (needsWidget) {
        nsHTMLContainerFrame::CreateViewForFrame(this, nsnull, PR_TRUE); 
        nsIView* view = GetView();

        if (!view->HasWidget())
           view->CreateWidget(kWidgetCID);   
    }
  }
  
  mMouseThrough = unset;

  UpdateMouseThrough();

  return rv;
}

NS_IMETHODIMP
nsLeafBoxFrame::AttributeChanged(PRInt32 aNameSpaceID,
                                 nsIAtom* aAttribute,
                                 PRInt32 aModType)
{
  nsresult rv = nsLeafFrame::AttributeChanged(aNameSpaceID, aAttribute,
                                              aModType);

  if (aAttribute == nsXULAtoms::mousethrough) 
    UpdateMouseThrough();

  return rv;
}

void nsLeafBoxFrame::UpdateMouseThrough()
{
  if (mContent) {
    static nsIContent::AttrValuesArray strings[] =
      {&nsXULAtoms::never, &nsXULAtoms::always, nsnull};
    switch (mContent->FindAttrValueIn(kNameSpaceID_None,
                                      nsXULAtoms::mousethrough,
                                      strings, eCaseMatters)) {
      case 0: mMouseThrough = never; break;
      case 1: mMouseThrough = always; break;
    }
  }
}

NS_IMETHODIMP
nsLeafBoxFrame::GetMouseThrough(PRBool& aMouseThrough)
{
  switch (mMouseThrough)
  {
    case always:
      aMouseThrough = PR_TRUE;
      return NS_OK;
    case never:
      aMouseThrough = PR_FALSE;      
      return NS_OK;
    case unset:
    {
      if (mParent && mParent->IsBoxFrame())
        return mParent->GetMouseThrough(aMouseThrough);
      else {
        aMouseThrough = PR_FALSE;      
        return NS_OK;
      }
    }
  }

  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
nsLeafBoxFrame::BuildDisplayList(nsDisplayListBuilder*   aBuilder,
                                 const nsRect&           aDirtyRect,
                                 const nsDisplayListSet& aLists)
{
  // REVIEW: GetFrameForPoint used to not report events for the background
  // layer, whereas this code will put an event receiver for this frame in the
  // BlockBorderBackground() list. But I don't see any need to preserve
  // that anomalous behaviour. The important thing I'm preserving is that
  // leaf boxes continue to receive events in the foreground layer.
  nsresult rv = DisplayBorderBackgroundOutline(aBuilder, aLists);
  NS_ENSURE_SUCCESS(rv, rv);

  if (!aBuilder->IsForEventDelivery() || !IsVisibleForPainting(aBuilder))
    return NS_OK;

  return aLists.Content()->AppendNewToTop(new (aBuilder)
      nsDisplayEventReceiver(this));
}

NS_IMETHODIMP
nsLeafBoxFrame::DidReflow(nsPresContext*           aPresContext,
                          const nsHTMLReflowState*  aReflowState,
                          nsDidReflowStatus         aStatus)
{
  PRBool isDirty = mState & NS_FRAME_IS_DIRTY;
  PRBool hasDirtyChildren = mState & NS_FRAME_HAS_DIRTY_CHILDREN;
  nsresult rv = nsFrame::DidReflow(aPresContext, aReflowState, aStatus);
  if (isDirty)
    mState |= NS_FRAME_IS_DIRTY;

  if (hasDirtyChildren)
    mState |= NS_FRAME_HAS_DIRTY_CHILDREN;

  return rv;

}


NS_IMETHODIMP
nsLeafBoxFrame::Reflow(nsPresContext*   aPresContext,
                     nsHTMLReflowMetrics&     aDesiredSize,
                     const nsHTMLReflowState& aReflowState,
                     nsReflowStatus&          aStatus)
{
  // This is mostly a copy of nsBoxFrame::Reflow().
  // We aren't able to share an implementation because of the frame
  // class hierarchy.  If you make changes here, please keep
  // nsBoxFrame::Reflow in sync.

  DO_GLOBAL_REFLOW_COUNT("nsLeafBoxFrame", aReflowState.reason);
  DISPLAY_REFLOW(aPresContext, this, aReflowState, aDesiredSize, aStatus);

  NS_ASSERTION(aReflowState.mComputedWidth >=0 && aReflowState.mComputedHeight >= 0, "Computed Size < 0");

#ifdef DO_NOISY_REFLOW
  printf("\n-------------Starting LeafBoxFrame Reflow ----------------------------\n");
  printf("%p ** nsLBF::Reflow %d R: ", this, myCounter++);
  switch (aReflowState.reason) {
    case eReflowReason_Initial:
      printf("Ini");break;
    case eReflowReason_Incremental:
      printf("Inc");break;
    case eReflowReason_Resize:
      printf("Rsz");break;
    case eReflowReason_StyleChange:
      printf("Sty");break;
    case eReflowReason_Dirty:
      printf("Drt ");
      break;
    default:printf("<unknown>%d", aReflowState.reason);break;
  }
  
  printSize("AW", aReflowState.availableWidth);
  printSize("AH", aReflowState.availableHeight);
  printSize("CW", aReflowState.mComputedWidth);
  printSize("CH", aReflowState.mComputedHeight);

  printf(" *\n");

#endif

  aStatus = NS_FRAME_COMPLETE;

  // create the layout state
  nsBoxLayoutState state(aPresContext, aReflowState, aDesiredSize);

  // coelesce reflows if we are root.
  state.HandleReflow(this);
  
  nsSize computedSize(aReflowState.mComputedWidth,aReflowState.mComputedHeight);

  nsMargin m;
  m = aReflowState.mComputedBorderPadding;

  //GetBorderAndPadding(m);

  // this happens sometimes. So lets handle it gracefully.
  if (aReflowState.mComputedHeight == 0) {
    nsSize minSize(0,0);
    GetMinSize(state, minSize);
    computedSize.height = minSize.height - m.top - m.bottom;
  }

  nsSize prefSize(0,0);

  // if we are told to layout intrinic then get our preferred size.
  if (computedSize.width == NS_INTRINSICSIZE || computedSize.height == NS_INTRINSICSIZE) {
     nsSize minSize(0,0);
     nsSize maxSize(0,0);
     GetPrefSize(state, prefSize);
     GetMinSize(state,  minSize);
     GetMaxSize(state,  maxSize);
     BoundsCheck(minSize, prefSize, maxSize);
  }

  // get our desiredSize
  if (aReflowState.mComputedWidth == NS_INTRINSICSIZE) {
    computedSize.width = prefSize.width;
  } else {
    computedSize.width += m.left + m.right;
  }

  if (aReflowState.mComputedHeight == NS_INTRINSICSIZE) {
    computedSize.height = prefSize.height;
  } else {
    computedSize.height += m.top + m.bottom;
  }

  // handle reflow state min and max sizes

  if (computedSize.width > aReflowState.mComputedMaxWidth)
    computedSize.width = aReflowState.mComputedMaxWidth;

  if (computedSize.height > aReflowState.mComputedMaxHeight)
    computedSize.height = aReflowState.mComputedMaxHeight;

  if (computedSize.width < aReflowState.mComputedMinWidth)
    computedSize.width = aReflowState.mComputedMinWidth;

  if (computedSize.height < aReflowState.mComputedMinHeight)
    computedSize.height = aReflowState.mComputedMinHeight;

  nsRect r(mRect.x, mRect.y, computedSize.width, computedSize.height);

  SetBounds(state, r);
 
  // layout our children
  Layout(state);
  
  // ok our child could have gotten bigger. So lets get its bounds
  
  // get the ascent
  nscoord ascent = mRect.height;

  // Only call GetAscent when not doing Initial reflow while in PP
  // or when it is Initial reflow while in PP and a chrome doc
  // If called again with initial reflow it crashes because the 
  // frames are fully constructed (I think).
  PRBool isChrome;
  PRBool isInitialPP = nsBoxFrame::IsInitialReflowForPrintPreview(state, isChrome);
  if (!isInitialPP || (isInitialPP && isChrome)) {
    GetAscent(state, ascent);
  }

  aDesiredSize.width  = mRect.width;
  aDesiredSize.height = mRect.height;
  aDesiredSize.ascent = ascent;
  aDesiredSize.descent = 0;

  // NS_FRAME_OUTSIDE_CHILDREN is set in SetBounds() above
  if (mState & NS_FRAME_OUTSIDE_CHILDREN) {
    nsRect* overflowArea = GetOverflowAreaProperty();
    NS_ASSERTION(overflowArea, "Failed to set overflow area property");
    aDesiredSize.mOverflowArea = *overflowArea;
  }

  // max sure the max element size reflects
  // our min width
  nscoord* maxElementWidth = state.GetMaxElementWidth();
  if (maxElementWidth)
  {
     nsSize minSize(0,0);
     GetMinSize(state,  minSize);

     if (mRect.width > minSize.width) {
       if (aReflowState.mComputedWidth == NS_INTRINSICSIZE) {
         *maxElementWidth = minSize.width;
       } else {
         *maxElementWidth = mRect.width;
       }
     } else {
        *maxElementWidth = mRect.width;
     }
  }
#ifdef DO_NOISY_REFLOW
  {
    printf("%p ** nsLBF(done) W:%d H:%d  ", this, aDesiredSize.width, aDesiredSize.height);

    if (maxElementWidth) {
      printf("MW:%d\n", *maxElementWidth); 
    } else {
      printf("MW:?\n"); 
    }

  }
#endif

  return NS_OK;
}

#ifdef DEBUG
NS_IMETHODIMP
nsLeafBoxFrame::GetFrameName(nsAString& aResult) const
{
  return MakeFrameName(NS_LITERAL_STRING("LeafBox"), aResult);
}
#endif

NS_IMETHODIMP_(nsrefcnt) 
nsLeafBoxFrame::AddRef(void)
{
  return NS_OK;
}

NS_IMETHODIMP_(nsrefcnt)
nsLeafBoxFrame::Release(void)
{
    return NS_OK;
}

NS_IMETHODIMP
nsLeafBoxFrame::CharacterDataChanged(nsPresContext* aPresContext,
                                     nsIContent*     aChild,
                                     PRBool          aAppend)
{
  NeedsRecalc();
  return nsLeafFrame::CharacterDataChanged(aPresContext, aChild, aAppend);
}

NS_IMETHODIMP
nsLeafBoxFrame::GetPrefSize(nsBoxLayoutState& aState, nsSize& aSize)
{
    return nsBox::GetPrefSize(aState, aSize);
}

NS_IMETHODIMP
nsLeafBoxFrame::GetMinSize(nsBoxLayoutState& aState, nsSize& aSize)
{
    return nsBox::GetMinSize(aState, aSize);
}

NS_IMETHODIMP
nsLeafBoxFrame::GetMaxSize(nsBoxLayoutState& aState, nsSize& aSize)
{
    return nsBox::GetMaxSize(aState, aSize);
}

NS_IMETHODIMP
nsLeafBoxFrame::GetFlex(nsBoxLayoutState& aState, nscoord& aFlex)
{
    return nsBox::GetFlex(aState, aFlex);
}

NS_IMETHODIMP
nsLeafBoxFrame::GetAscent(nsBoxLayoutState& aState, nscoord& aAscent)
{
    return nsBox::GetAscent(aState, aAscent);
}

NS_IMETHODIMP
nsLeafBoxFrame::NeedsRecalc()
{
    return nsBox::NeedsRecalc();
}

NS_IMETHODIMP
nsLeafBoxFrame::DoLayout(nsBoxLayoutState& aState)
{
    return nsBox::DoLayout(aState);
}

PRBool
nsLeafBoxFrame::HasStyleChange()
{
    return nsBox::HasStyleChange();
}

void
nsLeafBoxFrame::SetStyleChangeFlag(PRBool aDirty)
{
    nsBox::SetStyleChangeFlag(aDirty);
}

PRBool
nsLeafBoxFrame::GetWasCollapsed(nsBoxLayoutState& aState)
{
    return nsBox::GetWasCollapsed(aState);
}

void
nsLeafBoxFrame::SetWasCollapsed(nsBoxLayoutState& aState, PRBool aWas)
{
    nsBox::SetWasCollapsed(aState, aWas);
}
