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

/* struct containing the input to nsIFrame::Reflow */

#ifndef nsHTMLReflowState_h___
#define nsHTMLReflowState_h___

#include "nsMargin.h"
#include "nsStyleCoord.h"

class nsIFrame;
class nsPresContext;
class nsReflowPath;
class nsIRenderingContext;
class nsSpaceManager;
class nsLineLayout;
class nsIPercentHeightObserver;

struct nsStyleDisplay;
struct nsStyleVisibility;
struct nsStylePosition;
struct nsStyleBorder;
struct nsStyleMargin;
struct nsStylePadding;
struct nsStyleText;
struct nsHypotheticalBox;

/**
 * Constant used to indicate an unconstrained size.
 *
 * @see #Reflow()
 */
#define NS_UNCONSTRAINEDSIZE NS_MAXSIZE

/**
 * The reason the frame is being reflowed.
 *
 * XXX Should probably be a #define so it can be extended for specialized
 * reflow interfaces...
 *
 * @see nsHTMLReflowState
 */
enum nsReflowReason {
  eReflowReason_Initial = 0,       // initial reflow of a newly created frame
  eReflowReason_Incremental = 1,   // an incremental change has occurred. see the reflow command for details
  eReflowReason_Resize = 2,        // general request to determine a desired size
  eReflowReason_StyleChange = 3,   // request to reflow because of a style change. Note: you must reflow
                                   // all your child frames
  eReflowReason_Dirty = 4          // request to reflow because you and/or your children are dirty
};

/**
 * CSS Frame type. Included as part of the reflow state.
 */
typedef PRUint32  nsCSSFrameType;

#define NS_CSS_FRAME_TYPE_UNKNOWN         0
#define NS_CSS_FRAME_TYPE_INLINE          1
#define NS_CSS_FRAME_TYPE_BLOCK           2  /* block-level in normal flow */
#define NS_CSS_FRAME_TYPE_FLOATING        3
#define NS_CSS_FRAME_TYPE_ABSOLUTE        4
#define NS_CSS_FRAME_TYPE_INTERNAL_TABLE  5  /* row group frame, row frame, cell frame, ... */

/**
 * Bit-flag that indicates whether the element is replaced. Applies to inline,
 * block-level, floating, and absolutely positioned elements
 */
#define NS_CSS_FRAME_TYPE_REPLACED        0x8000

/**
 * Helper macros for telling whether items are replaced
 */
#define NS_FRAME_IS_REPLACED(_ft) \
  (NS_CSS_FRAME_TYPE_REPLACED == ((_ft) & NS_CSS_FRAME_TYPE_REPLACED))

#define NS_FRAME_REPLACED(_ft) \
  (NS_CSS_FRAME_TYPE_REPLACED | (_ft))

/**
 * A macro to extract the type. Masks off the 'replaced' bit-flag
 */
#define NS_FRAME_GET_TYPE(_ft) \
  ((_ft) & ~NS_CSS_FRAME_TYPE_REPLACED)

#define NS_INTRINSICSIZE    NS_UNCONSTRAINEDSIZE
#define NS_SHRINKWRAPWIDTH  NS_UNCONSTRAINEDSIZE
#define NS_AUTOHEIGHT       NS_UNCONSTRAINEDSIZE
#define NS_AUTOMARGIN       NS_UNCONSTRAINEDSIZE
#define NS_AUTOOFFSET       NS_UNCONSTRAINEDSIZE
// NOTE: there are assumptions all over that these have the same value, namely NS_UNCONSTRAINEDSIZE
//       if any are changed to be a value other than NS_UNCONSTRAINEDSIZE
//       at least update AdjustComputedHeight/Width and test ad nauseum

/**
 * Reflow state passed to a frame during reflow.
 *
 * @see nsIFrame#Reflow()
 */
struct nsHTMLReflowState {
  // the reflow states are linked together. this is the pointer to the
  // parent's reflow state
  const nsHTMLReflowState* parentReflowState;

  // the frame being reflowed
  nsIFrame*           frame;

  // the reason for the reflow
  nsReflowReason      reason;

  // the incremental reflow path, when the reflow reason is
  // eReflowReason_Incremental. Specifically, this corresponds to the
  // portion of the incremental reflow path from `frame' down. Note
  // that it is safe to assume that this is non-null: we maintain the
  // invariant that it contains a valid nsReflowPath pointer when
  // reason == eReflowReason_Incremental.
  nsReflowPath        *path;

  // the available width in which to reflow the frame. The space
  // represents the amount of room for the frame's border, padding,
  // and content area (not the margin area. The parent frame deals
  // with the child frame's margins). The frame size you choose should
  // fit within the available width.
  nscoord              availableWidth;

  // A value of NS_UNCONSTRAINEDSIZE for the available height means
  // you can choose whatever size you want. In galley mode the
  // available height is always NS_UNCONSTRAINEDSIZE, and only page
  // mode involves a constrained height. The element's the top border
  // and padding, and content, must fit. If the element is complete
  // after reflow then its bottom border, padding and margin (and
  // similar for its complete ancestors) will need to fit in this
  // height.
  nscoord              availableHeight;

  // rendering context to use for measurement
  nsIRenderingContext* rendContext;

  // The type of frame, from css's perspective. This value is
  // initialized by the Init method below.
  nsCSSFrameType   mFrameType;

  // pointer to the space manager associated with this area
  nsSpaceManager* mSpaceManager;

  // LineLayout object (only for inline reflow; set to NULL otherwise)
  nsLineLayout*    mLineLayout;

  // The appropriate reflow state for the containing block (for
  // percentage widths, etc.) of this reflow state's frame.
  const nsHTMLReflowState *mCBReflowState;

  // The computed width specifies the frame's content area width, and it does
  // not apply to inline non-replaced elements
  //
  // For replaced inline frames, a value of NS_INTRINSICSIZE means you should
  // use your intrinsic width as the computed width
  //
  // For block-level frames, the computed width is based on the width of the
  // containing block, the margin/border/padding areas, and the min/max width.
  // A value of NS_SHRINKWRAPWIDTH means that you should choose a width based
  // on your content. The width may be as large as the specified maximum width
  // (see mComputedMaxWidth).
  nscoord          mComputedWidth; 

  // The computed height specifies the frame's content height, and it does
  // not apply to inline non-replaced elements
  //
  // For replaced inline frames, a value of NS_INTRINSICSIZE means you should
  // use your intrinsic height as the computed height
  //
  // For non-replaced block-level frames in the flow and floated, a value of
  // NS_AUTOHEIGHT means you choose a height to shrink wrap around the normal
  // flow child frames. The height must be within the limit of the min/max
  // height if there is such a limit
  //
  // For replaced block-level frames, a value of NS_INTRINSICSIZE
  // means you use your intrinsic height as the computed height
  nscoord          mComputedHeight;

  // Computed margin values
  nsMargin         mComputedMargin;

  // Cached copy of the border values
  nsMargin         mComputedBorderPadding;

  // Computed padding values
  nsMargin         mComputedPadding;

  // Computed values for 'left/top/right/bottom' offsets. Only applies to
  // 'positioned' elements
  nsMargin         mComputedOffsets;

  // Computed values for 'min-width/max-width' and 'min-height/max-height'
  nscoord          mComputedMinWidth, mComputedMaxWidth;
  nscoord          mComputedMinHeight, mComputedMaxHeight;

  // Cached pointers to the various style structs used during intialization
  const nsStyleDisplay*    mStyleDisplay;
  const nsStyleVisibility* mStyleVisibility;
  const nsStylePosition*   mStylePosition;
  const nsStyleBorder*     mStyleBorder;
  const nsStyleMargin*     mStyleMargin;
  const nsStylePadding*    mStylePadding;
  const nsStyleText*       mStyleText;

  // a frame (e.g. nsTableCellFrame) which may need to generate a special 
  // reflow for percent height calculations 
  nsIPercentHeightObserver* mPercentHeightObserver;

  // a frame (e.g. nsTableFrame) which initiates a special reflow for percent height calculations 
  nsIFrame* mPercentHeightReflowInitiator;

  // CSS margin collapsing sometimes requires us to reflow
  // optimistically assuming that margins collapse to see if clearance
  // is required. When we discover that clearance is required, we
  // store the frame in which clearance was discovered to the location
  // requested here.
  nsIFrame** mDiscoveredClearance;

  // This value keeps track of how deeply nested a given reflow state
  // is from the top of the frame tree.
  PRInt16 mReflowDepth;

  struct ReflowStateFlags {
    PRUint16 mSpecialHeightReflow:1; // used by tables to communicate special reflow (in process) to handle
                                     // percent height frames inside cells which may not have computed heights
    PRUint16 mNextInFlowUntouched:1; // nothing in the frame's next-in-flow (or its descendants)
                                     // is changing
    PRUint16 mIsTopOfPage:1;         // is the current context at the top of a page?
    PRUint16 mBlinks:1;              // Keep track of text-decoration: blink
    PRUint16 mVisualBidiFormControl:1; // Keep track of descendants of form controls on Visual Bidi pages
    PRUint16 mHasClearance:1;        // Block has clearance
    PRUint16 mAssumingHScrollbar:1;  // parent frame is an nsIScrollableFrame and it
                                     // is assuming a horizontal scrollbar
    PRUint16 mAssumingVScrollbar:1;  // parent frame is an nsIScrollableFrame and it
                                     // is assuming a vertical scrollbar
  } mFlags;

#ifdef IBMBIDI
  nscoord mRightEdge;
#endif

#ifdef DEBUG
  // hook for attaching debug info (e.g. tables may attach a timer during reflow)
  void* mDebugHook;

  static const char* ReasonToString(nsReflowReason aReason);
#endif

  // Note: The copy constructor is written by the compiler automatically. You
  // can use that and then override specific values if you want, or you can
  // call Init as desired...

  // Initialize a <b>root</b> reflow state with a rendering context to
  // use for measuring things.
  nsHTMLReflowState(nsPresContext*          aPresContext,
                    nsIFrame*                aFrame,
                    nsReflowReason           aReason,
                    nsIRenderingContext*     aRenderingContext,
                    const nsSize&            aAvailableSpace);

  // Initialize a <b>root</b> reflow state for an <b>incremental</b>
  // reflow.
  nsHTMLReflowState(nsPresContext*          aPresContext,
                    nsIFrame*                aFrame,
                    nsReflowPath*            aReflowPath,
                    nsIRenderingContext*     aRenderingContext,
                    const nsSize&            aAvailableSpace);

  // Initialize a reflow state for a child frames reflow. Some state
  // is copied from the parent reflow state; the remaining state is
  // computed. 
  nsHTMLReflowState(nsPresContext*          aPresContext,
                    const nsHTMLReflowState& aParentReflowState,
                    nsIFrame*                aFrame,
                    const nsSize&            aAvailableSpace,
                    nsReflowReason           aReason, 
                    PRBool                   aInit = PR_TRUE);

  // Same as the previous except that the reason is taken from the
  // parent's reflow state.
  nsHTMLReflowState(nsPresContext*          aPresContext,
                    const nsHTMLReflowState& aParentReflowState,
                    nsIFrame*                aFrame,
                    const nsSize&            aAvailableSpace);

  // Used when you want to override the default containing block
  // width and height. Used by absolute positioning code
  nsHTMLReflowState(nsPresContext*          aPresContext,
                    const nsHTMLReflowState& aParentReflowState,
                    nsIFrame*                aFrame,
                    const nsSize&            aAvailableSpace,
                    nscoord                  aContainingBlockWidth,
                    nscoord                  aContainingBlockHeight,
                    nsReflowReason           aReason);

  // This method initializes various data members. It is automatically
  // called by the various constructors
  void Init(nsPresContext* aPresContext,
            nscoord         aContainingBlockWidth = -1,
            nscoord         aContainingBlockHeight = -1,
            nsMargin*       aBorder = nsnull,
            nsMargin*       aPadding = nsnull);
  /**
   * Find the content width of the containing block of aReflowState
   */
  static nscoord
    GetContainingBlockContentWidth(const nsHTMLReflowState* aReflowState);

  /**
   * Adjust content MEW take into account the settings of the CSS
   * 'width', 'min-width' and 'max-width' properties.
   */
  nscoord AdjustIntrinsicMinContentWidthForStyle(nscoord aWidth) const;
  /**
   * Adjust content maximum-width take into account the settings of
   * the CSS 'width', 'min-width' and 'max-width' properties.
   */
  nscoord AdjustIntrinsicContentWidthForStyle(nscoord aWidth) const;

  /**
   * Find the containing block of aFrame.  This may return null if
   * there isn't one (but that should really only happen for root
   * frames).
   */
  static nsIFrame* GetContainingBlockFor(const nsIFrame* aFrame);

  /**
   * Get the page box reflow state, starting from a frames
   * <B>parent</B> reflow state (the parent reflow state may or may not end
   * up being the containing block reflow state)
   */
  static const nsHTMLReflowState*
    GetPageBoxReflowState(const nsHTMLReflowState* aParentRS);

  /**
   * Compute the border plus padding for <TT>aFrame</TT>. If a
   * percentage needs to be computed it will be computed by finding
   * the containing block, use GetContainingBlockReflowState.
   * aParentReflowState is aFrame's
   * parent's reflow state. The resulting computed border plus padding
   * is returned in aResult.
   */
  static void ComputeBorderPaddingFor(nsIFrame* aFrame,
                                      const nsHTMLReflowState* aParentRS,
                                      nsMargin& aResult);

  /**
   * Calculate the raw line-height property for the given frame. The return
   * value, if line-height was applied and is valid will be >= 0. Otherwise,
   * the return value will be <0 which is illegal (CSS2 spec: section 10.8.1).
   */
  static nscoord CalcLineHeight(nsPresContext* aPresContext,
                                nsIRenderingContext* aRenderingContext,
                                nsIFrame* aFrame);

  void InitFrameType();

  void ComputeContainingBlockRectangle(nsPresContext*          aPresContext,
                                       const nsHTMLReflowState* aContainingBlockRS,
                                       nscoord&                 aContainingBlockWidth,
                                       nscoord&                 aContainingBlockHeight);

  void CalculateBlockSideMargins(nscoord aAvailWidth,
                                 nscoord aComputedWidth);

  /**
   * Apply the mComputed(Min/Max)(Width/Height) values to the content
   * size computed so far. If a passed-in pointer is null, we skip
   * adjusting that dimension.
   */
  void ApplyMinMaxConstraints(nscoord* aContentWidth, nscoord* aContentHeight) const;

protected:

  void InitCBReflowState();

  void InitConstraints(nsPresContext* aPresContext,
                       nscoord         aContainingBlockWidth,
                       nscoord         aContainingBlockHeight,
                       nsMargin*       aBorder,
                       nsMargin*       aPadding);

  void CalculateHypotheticalBox(nsPresContext*    aPresContext,
                                nsIFrame*          aPlaceholderFrame,
                                nsIFrame*          aContainingBlock,
                                nsMargin&          aBlockContentArea,
                                const nsHTMLReflowState* cbrs,
                                nsHypotheticalBox& aHypotheticalBox);

  void InitAbsoluteConstraints(nsPresContext* aPresContext,
                               const nsHTMLReflowState* cbrs,
                               nscoord aContainingBlockWidth,
                               nscoord aContainingBlockHeight);

  void ComputeRelativeOffsets(const nsHTMLReflowState* cbrs,
                              nscoord aContainingBlockWidth,
                              nscoord aContainingBlockHeight);

  void ComputeBlockBoxData(nsPresContext* aPresContext,
                           const nsHTMLReflowState* cbrs,
                           nsStyleUnit aWidthUnit,
                           nsStyleUnit aHeightUnit,
                           nscoord aContainingBlockWidth,
                           nscoord aContainingBlockHeight);

  void ComputeHorizontalValue(nscoord aContainingBlockWidth,
                                     nsStyleUnit aUnit,
                                     const nsStyleCoord& aCoord,
                                     nscoord& aResult);

  void ComputeVerticalValue(nscoord aContainingBlockHeight,
                                   nsStyleUnit aUnit,
                                   const nsStyleCoord& aCoord,
                                   nscoord& aResult);

  // Computes margin values from the specified margin style information, and
  // fills in the mComputedMargin member
  void ComputeMargin(nscoord aContainingBlockWidth,
                     const nsHTMLReflowState* aContainingBlockRS);
  
  // Computes padding values from the specified padding style information, and
  // fills in the mComputedPadding member
  void ComputePadding(nscoord aContainingBlockWidth,
                      const nsHTMLReflowState* aContainingBlockRS);

  // Calculates the computed values for the 'min-Width', 'max-Width',
  // 'min-Height', and 'max-Height' properties, and stores them in the assorted
  // data members
  void ComputeMinMaxValues(nscoord                  aContainingBlockWidth,
                           nscoord                  aContainingBlockHeight,
                           const nsHTMLReflowState* aContainingBlockRS);

  nscoord CalculateHorizBorderPaddingMargin(nscoord aContainingBlockWidth);

  // Adjust Computed sizes for Min/Max Width and box-Sizing (if
  // aAdjustForBoxSizing is true)  
  // - guarantees that the computed height/width will be non-negative
  //   If the value goes negative (because the padding or border is greater than
  //   the width/height and it is removed due to box sizing) then it is driven to 0
  void AdjustComputedHeight(PRBool aAdjustForBoxSizing);
  void AdjustComputedWidth(PRBool aAdjustForBoxSizing);

#ifdef IBMBIDI
  /**
   * Test whether the frame is a form control in a visual Bidi page.
   * This is necessary for backwards-compatibility, because most visual
   * pages use logical order for form controls so that they will
   * display correctly on native widgets in OSs with Bidi support
   * @param aPresContext the pres context
   * @return whether the frame is a BIDI form control
   */
  PRBool IsBidiFormControl(nsPresContext* aPresContext);
#endif
};

#endif /* nsHTMLReflowState_h___ */

