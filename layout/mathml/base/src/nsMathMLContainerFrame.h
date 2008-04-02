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
 * The Original Code is Mozilla MathML Project.
 *
 * The Initial Developer of the Original Code is
 * The University Of Queensland.
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Roger B. Sidje <rbs@maths.uq.edu.au>
 *   David J. Fiddes <D.J.Fiddes@hw.ac.uk>
 *   Shyjan Mahamud <mahamud@cs.cmu.edu> (added TeX rendering rules)
 *   Karl Tomlinson <karlt+@karlt.net>, Mozilla Corporation
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

#ifndef nsMathMLContainerFrame_h___
#define nsMathMLContainerFrame_h___

#include "nsCOMPtr.h"
#include "nsHTMLContainerFrame.h"
#include "nsBlockFrame.h"
#include "nsInlineFrame.h"
#include "nsMathMLAtoms.h"
#include "nsMathMLOperators.h"
#include "nsMathMLChar.h"
#include "nsMathMLFrame.h"
#include "nsMathMLParts.h"

/*
 * Base class for MathML container frames. It acts like an inferred 
 * mrow. By default, this frame uses its Reflow() method to lay its 
 * children horizontally and ensure that their baselines are aligned.
 * The Reflow() method relies upon Place() to position children.
 * By overloading Place() in derived classes, it is therefore possible
 * to position children in various customized ways.
 */

// Options for the preferred size at which to stretch our stretchy children 
#define STRETCH_CONSIDER_ACTUAL_SIZE    0x00000001 // just use our current size
#define STRETCH_CONSIDER_EMBELLISHMENTS 0x00000002 // size calculations include embellishments

class nsMathMLContainerFrame : public nsHTMLContainerFrame,
                               public nsMathMLFrame {
  friend class nsMathMLmfencedFrame;
public:
  nsMathMLContainerFrame(nsStyleContext* aContext) : nsHTMLContainerFrame(aContext) {}

  NS_DECL_ISUPPORTS_INHERITED

  // --------------------------------------------------------------------------
  // Overloaded nsMathMLFrame methods -- see documentation in nsIMathMLFrame.h

  NS_IMETHOD
  Stretch(nsIRenderingContext& aRenderingContext,
          nsStretchDirection   aStretchDirection,
          nsBoundingMetrics&   aContainerSize,
          nsHTMLReflowMetrics& aDesiredStretchSize);

  NS_IMETHOD
  Place(nsIRenderingContext& aRenderingContext,
        PRBool               aPlaceOrigin,
        nsHTMLReflowMetrics& aDesiredSize);

  NS_IMETHOD
  UpdatePresentationDataFromChildAt(PRInt32         aFirstIndex,
                                    PRInt32         aLastIndex,
                                    PRUint32        aFlagsValues,
                                    PRUint32        aFlagsToUpdate)
  {
    PropagatePresentationDataFromChildAt(this, aFirstIndex, aLastIndex,
      aFlagsValues, aFlagsToUpdate);
    return NS_OK;
  }
  
  // helper to set the "increment script level" flag on the element belonging
  // to a child frame given by aChildIndex.
  // When this flag is set, the style system will increment the scriptlevel
  // for the child element. This is needed for situations where the style system
  // cannot itself determine the scriptlevel (mfrac, munder, mover, munderover).
  // This should be called during reflow. We set the flag and if it changed,
  // we request appropriate restyling and also queue a post-reflow callback
  // to ensure that restyle and reflow happens immediately after the current
  // reflow.
  void
  SetIncrementScriptLevel(PRInt32 aChildIndex, PRBool aIncrement);

  // --------------------------------------------------------------------------
  // Overloaded nsHTMLContainerFrame methods -- see documentation in nsIFrame.h

  virtual PRBool IsFrameOfType(PRUint32 aFlags) const
  {
    return !(aFlags & nsIFrame::eLineParticipant) &&
      nsHTMLContainerFrame::IsFrameOfType(aFlags & ~(nsIFrame::eMathML));
  }

  virtual PRIntn GetSkipSides() const { return 0; }

  NS_IMETHOD
  AppendFrames(nsIAtom*        aListName,
               nsIFrame*       aFrameList);

  NS_IMETHOD
  InsertFrames(nsIAtom*        aListName,
               nsIFrame*       aPrevFrame,
               nsIFrame*       aFrameList);

  NS_IMETHOD
  RemoveFrame(nsIAtom*        aListName,
              nsIFrame*       aOldFrame);

  /**
   * Both GetMinWidth and GetPrefWidth return whatever
   * GetIntrinsicWidth returns.
   */
  virtual nscoord GetMinWidth(nsIRenderingContext *aRenderingContext);
  virtual nscoord GetPrefWidth(nsIRenderingContext *aRenderingContext);

  /**
   * Return the intrinsic width of the frame's content area.
   */
  virtual nscoord GetIntrinsicWidth(nsIRenderingContext *aRenderingContext);

  NS_IMETHOD
  Reflow(nsPresContext*          aPresContext,
         nsHTMLReflowMetrics&     aDesiredSize,
         const nsHTMLReflowState& aReflowState,
         nsReflowStatus&          aStatus);

  NS_IMETHOD
  WillReflow(nsPresContext* aPresContext)
  {
    mPresentationData.flags &= ~NS_MATHML_ERROR;
    return nsHTMLContainerFrame::WillReflow(aPresContext);
  }

  NS_IMETHOD
  DidReflow(nsPresContext*           aPresContext,
            const nsHTMLReflowState*  aReflowState,
            nsDidReflowStatus         aStatus)

  {
    mPresentationData.flags &= ~NS_MATHML_STRETCH_DONE;
    return nsHTMLContainerFrame::DidReflow(aPresContext, aReflowState, aStatus);
  }

  NS_IMETHOD BuildDisplayList(nsDisplayListBuilder*   aBuilder,
                              const nsRect&           aDirtyRect,
                              const nsDisplayListSet& aLists);

  // Notification when an attribute is changed. The MathML module uses the
  // following paradigm:
  //
  // 1. If the MathML frame class doesn't have any cached automatic data that
  //    depends on the attribute: we just reflow (e.g., this happens with <msub>,
  //    <msup>, <mmultiscripts>, etc). This is the default behavior implemented
  //    by this base class.
  //
  // 2. If the MathML frame class has cached automatic data that depends on
  //    the attribute:
  //    2a. If the automatic data to update resides only within the descendants,
  //        we just re-layout them using ReLayoutChildren(this);
  //        (e.g., this happens with <ms>).
  //    2b. If the automatic data to update affects us in some way, we ask our parent
  //        to re-layout its children using ReLayoutChildren(mParent);
  //        Therefore, there is an overhead here in that our siblings are re-laid
  //        too (e.g., this happens with <mstyle>, <munder>, <mover>, <munderover>). 
  NS_IMETHOD
  AttributeChanged(PRInt32         aNameSpaceID,
                   nsIAtom*        aAttribute,
                   PRInt32         aModType);

  // --------------------------------------------------------------------------
  // Additional methods 

  // helper to re-sync the automatic data in our children and notify our parent to
  // reflow us when changes (e.g., append/insert/remove) happen in our child list
  virtual nsresult
  ChildListChanged(PRInt32 aModType);

  // helper to get the preferred size that a container frame should use to fire
  // the stretch on its stretchy child frames.
  virtual void
  GetPreferredStretchSize(nsIRenderingContext& aRenderingContext,
                          PRUint32             aOptions,
                          nsStretchDirection   aStretchDirection,
                          nsBoundingMetrics&   aPreferredStretchSize);

  // error handlers to provide a visual feedback to the user when an error
  // (typically invalid markup) was encountered during reflow.
  virtual nsresult
  ReflowError(nsIRenderingContext& aRenderingContext,
              nsHTMLReflowMetrics& aDesiredSize);

  // helper method to reflow a child frame. We are inline frames, and we don't
  // know our positions until reflow is finished. That's why we ask the
  // base method not to worry about our position.
  nsresult 
  ReflowChild(nsIFrame*                aKidFrame,
              nsPresContext*          aPresContext,
              nsHTMLReflowMetrics&     aDesiredSize,
              const nsHTMLReflowState& aReflowState,
              nsReflowStatus&          aStatus);

  // helper to add the inter-spacing when <math> is the immediate parent.
  // Since we don't (yet) handle the root <math> element ourselves, we need to
  // take special care of the inter-frame spacing on elements for which <math>
  // is the direct xml parent. This function will be repeatedly called from
  // left to right on the childframes of <math>, and by so doing it will
  // emulate the spacing that would have been done by a <mrow> container.
  // e.g., it fixes <math> <mi>f</mi> <mo>q</mo> <mi>f</mi> <mo>I</mo> </math>
  virtual nscoord
  FixInterFrameSpacing(nsHTMLReflowMetrics& aDesiredSize);

  // helper method to complete the post-reflow hook and ensure that embellished
  // operators don't terminate their Reflow without receiving a Stretch command.
  virtual nsresult
  FinalizeReflow(nsIRenderingContext& aRenderingContext,
                 nsHTMLReflowMetrics& aDesiredSize);

  // Record metrics of a child frame for recovery through the following method
  static void
  SaveReflowAndBoundingMetricsFor(nsIFrame*                  aFrame,
                                  const nsHTMLReflowMetrics& aReflowMetrics,
                                  const nsBoundingMetrics&   aBoundingMetrics);

  // helper method to facilitate getting the reflow and bounding metrics of a
  // child frame.  The argument aMathMLFrameType, when non null, will return
  // the 'type' of the frame, which is used to determine the inter-frame
  // spacing.
  // IMPORTANT: This function is only meant to be called in Place() methods as
  // the information is available only when set up with the above method
  // during Reflow/Stretch() and GetPrefWidth().
  static void
  GetReflowAndBoundingMetricsFor(nsIFrame*            aFrame,
                                 nsHTMLReflowMetrics& aReflowMetrics,
                                 nsBoundingMetrics&   aBoundingMetrics,
                                 eMathMLFrameType*    aMathMLFrameType = nsnull);

  // helper method to clear metrics saved with
  // SaveReflowAndBoundingMetricsFor() from all child frames.
  void ClearSavedChildMetrics();

  // helper to let the update of presentation data pass through
  // a subtree that may contain non-MathML container frames
  static void
  PropagatePresentationDataFor(nsIFrame*       aFrame,
                               PRUint32        aFlagsValues,
                               PRUint32        aFlagsToUpdate);

  static void
  PropagatePresentationDataFromChildAt(nsIFrame*       aParentFrame,
                                       PRInt32         aFirstChildIndex,
                                       PRInt32         aLastChildIndex,
                                       PRUint32        aFlagsValues,
                                       PRUint32        aFlagsToUpdate);

  // helper to let the rebuild of automatic data (presentation data
  // and embellishement data) walk through a subtree that may contain
  // non-MathML container frames. Note that this method re-builds the
  // automatic data in the children -- not in aParentFrame itself (except
  // for those particular operations that the parent frame may do in its
  // TransmitAutomaticData()). The reason it works this way is because
  // a container frame knows what it wants for its children, whereas children
  // have no clue who their parent is. For example, it is <mfrac> who knows
  // that its children have to be in scriptsizes, and has to transmit this
  // information to them. Hence, when changes occur in a child frame, the child
  // has to request the re-build from its parent. Unfortunately, the extra cost
  // for this is that it will re-sync in the siblings of the child as well.
  static void
  RebuildAutomaticDataForChildren(nsIFrame* aParentFrame);

  // helper to blow away the automatic data cached in a frame's subtree and
  // re-layout its subtree to reflect changes that may have happen. In the
  // event where aParentFrame isn't a MathML frame, it will first walk up to
  // the ancestor that is a MathML frame, and re-layout from there -- this is
  // to guarantee that automatic data will be rebuilt properly. Note that this
  // method re-builds the automatic data in the children -- not in the parent
  // frame itself (except for those particular operations that the parent frame
  // may do do its TransmitAutomaticData()). @see RebuildAutomaticDataForChildren
  //
  // aBits are the bits to pass to FrameNeedsReflow() when we call it.
  static nsresult
  ReLayoutChildren(nsIFrame* aParentFrame, nsFrameState aBits);

protected:
  // Helper method which positions child frames as an <mrow> on given baseline
  // y = aBaseline starting from x = aOffsetX, calling FinishReflowChild()
  // on the frames.
  void
  PositionRowChildFrames(nscoord aOffsetX, nscoord aBaseline);

  // A variant on FinishAndStoreOverflow() that uses the union of child
  // overflows, the frame bounds, and mBoundingMetrics to set and store the
  // overflow.
  void GatherAndStoreOverflow(nsHTMLReflowMetrics* aMetrics);

  /**
   * Call DidReflow() if the NS_FRAME_IN_REFLOW frame bit is set on aFirst and
   * all its next siblings up to, but not including, aStop.
   * aStop == nsnull meaning all next siblings with the bit set.
   * The method does nothing if aFirst == nsnull.
   */
  void DidReflowChildren(nsIFrame* aFirst, nsIFrame* aStop = nsnull);

private:
  class RowChildFrameIterator;
  friend class RowChildFrameIterator;
};


// --------------------------------------------------------------------------
// Currently, to benefit from line-breaking inside the <math> element, <math> is
// simply mapping to nsBlockFrame or nsInlineFrame.
// A separate implemention needs to provide:
// 1) line-breaking
// 2) proper inter-frame spacing
// 3) firing of Stretch() (in which case FinalizeReflow() would have to be cleaned)
// Issues: If/when mathml becomes a pluggable component, the separation will be needed.
class nsMathMLmathBlockFrame : public nsBlockFrame {
public:
  friend nsIFrame* NS_NewMathMLmathBlockFrame(nsIPresShell* aPresShell,
          nsStyleContext* aContext, PRUint32 aFlags);

  // beware, mFrames is not set by nsBlockFrame
  // cannot use mFrames{.FirstChild()|.etc} since the block code doesn't set mFrames
  NS_IMETHOD
  SetInitialChildList(nsIAtom*        aListName,
                      nsIFrame*       aChildList)
  {
    NS_ASSERTION(!aListName, "unexpected frame list");
    nsresult rv = nsBlockFrame::SetInitialChildList(aListName, aChildList);
    // re-resolve our subtree to set any mathml-expected data
    nsMathMLContainerFrame::RebuildAutomaticDataForChildren(this);
    return rv;
  }

  NS_IMETHOD
  AppendFrames(nsIAtom*        aListName,
               nsIFrame*       aFrameList)
  {
    NS_ASSERTION(!aListName || nsGkAtoms::nextBidi == aListName,
                 "unexpected frame list");
    nsresult rv = nsBlockFrame::AppendFrames(aListName, aFrameList);
    if (NS_LIKELY(!aListName))
      nsMathMLContainerFrame::ReLayoutChildren(this,
                                               NS_FRAME_HAS_DIRTY_CHILDREN);
    return rv;
  }

  NS_IMETHOD
  InsertFrames(nsIAtom*        aListName,
               nsIFrame*       aPrevFrame,
               nsIFrame*       aFrameList)
  {
    NS_ASSERTION(!aListName || nsGkAtoms::nextBidi == aListName,
                 "unexpected frame list");
    nsresult rv = nsBlockFrame::InsertFrames(aListName, aPrevFrame, aFrameList);
    if (NS_LIKELY(!aListName))
      nsMathMLContainerFrame::ReLayoutChildren(this,
                                               NS_FRAME_HAS_DIRTY_CHILDREN);
    return rv;
  }

  NS_IMETHOD
  RemoveFrame(nsIAtom*        aListName,
              nsIFrame*       aOldFrame)
  {
    NS_ASSERTION(!aListName || nsGkAtoms::nextBidi == aListName,
                 "unexpected frame list");
    nsresult rv = nsBlockFrame::RemoveFrame(aListName, aOldFrame);
    if (NS_LIKELY(!aListName))
      nsMathMLContainerFrame::ReLayoutChildren(this,
                                               NS_FRAME_HAS_DIRTY_CHILDREN);
    return rv;
  }

  virtual PRBool IsFrameOfType(PRUint32 aFlags) const {
    return nsBlockFrame::IsFrameOfType(aFlags & ~(nsIFrame::eMathML));
  }

protected:
  nsMathMLmathBlockFrame(nsStyleContext* aContext) : nsBlockFrame(aContext) {
    // We should always have a space manager.  Not that things can really try
    // to float out of us anyway, but we need one for line layout.
    AddStateBits(NS_BLOCK_SPACE_MGR);
  }
  virtual ~nsMathMLmathBlockFrame() {}
};

// --------------

class nsMathMLmathInlineFrame : public nsInlineFrame {
public:
  friend nsIFrame* NS_NewMathMLmathInlineFrame(nsIPresShell* aPresShell, nsStyleContext* aContext);

  NS_IMETHOD
  SetInitialChildList(nsIAtom*        aListName,
                      nsIFrame*       aChildList)
  {
    NS_ASSERTION(!aListName, "unexpected frame list");
    nsresult rv = nsInlineFrame::SetInitialChildList(aListName, aChildList);
    // re-resolve our subtree to set any mathml-expected data
    nsMathMLContainerFrame::RebuildAutomaticDataForChildren(this);
    return rv;
  }

  NS_IMETHOD
  AppendFrames(nsIAtom*        aListName,
               nsIFrame*       aFrameList)
  {
    NS_ASSERTION(!aListName || nsGkAtoms::nextBidi == aListName,
                 "unexpected frame list");
    nsresult rv = nsInlineFrame::AppendFrames(aListName, aFrameList);
    if (NS_LIKELY(!aListName))
      nsMathMLContainerFrame::ReLayoutChildren(this,
                                               NS_FRAME_HAS_DIRTY_CHILDREN);
    return rv;
  }

  NS_IMETHOD
  InsertFrames(nsIAtom*        aListName,
               nsIFrame*       aPrevFrame,
               nsIFrame*       aFrameList)
  {
    NS_ASSERTION(!aListName || nsGkAtoms::nextBidi == aListName,
                 "unexpected frame list");
    nsresult rv = nsInlineFrame::InsertFrames(aListName, aPrevFrame, aFrameList);
    if (NS_LIKELY(!aListName))
      nsMathMLContainerFrame::ReLayoutChildren(this,
                                               NS_FRAME_HAS_DIRTY_CHILDREN);
    return rv;
  }

  NS_IMETHOD
  RemoveFrame(nsIAtom*        aListName,
              nsIFrame*       aOldFrame)
  {
    NS_ASSERTION(!aListName || nsGkAtoms::nextBidi == aListName,
                 "unexpected frame list");
    nsresult rv = nsInlineFrame::RemoveFrame(aListName, aOldFrame);
    if (NS_LIKELY(!aListName))
      nsMathMLContainerFrame::ReLayoutChildren(this,
                                               NS_FRAME_HAS_DIRTY_CHILDREN);
    return rv;
  }

  virtual PRBool IsFrameOfType(PRUint32 aFlags) const {
    return nsInlineFrame::IsFrameOfType(aFlags & ~(nsIFrame::eMathML));
  }

protected:
  nsMathMLmathInlineFrame(nsStyleContext* aContext) : nsInlineFrame(aContext) {}
  virtual ~nsMathMLmathInlineFrame() {}
};

#endif /* nsMathMLContainerFrame_h___ */
