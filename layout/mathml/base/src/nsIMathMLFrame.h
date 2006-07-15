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
//#define SHOW_BOUNDING_BOX 1
#ifndef nsIMathMLFrame_h___
#define nsIMathMLFrame_h___

#include "nsIRenderingContext.h"
#include "nsIFrame.h"

struct nsPresentationData;
struct nsEmbellishData;
struct nsHTMLReflowMetrics;

// IID for the nsIMathMLFrame interface (the IID was taken from IIDS.h) 
/* a6cf9113-15b3-11d2-932e-00805f8add32 */
#define NS_IMATHMLFRAME_IID   \
{ 0xa6cf9113, 0x15b3, 0x11d2, \
  { 0x93, 0x2e, 0x00, 0x80, 0x5f, 0x8a, 0xdd, 0x32 } }

static NS_DEFINE_IID(kIMathMLFrameIID, NS_IMATHMLFRAME_IID);

// Abstract base class that provides additional methods for MathML frames
class nsIMathMLFrame : public nsISupports {
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IMATHMLFRAME_IID)

 /* SUPPORT FOR PRECISE POSITIONING */
 /*====================================================================*/
 
 /* Metrics that _exactly_ enclose the text of the frame.
  * The frame *must* have *already* being reflowed, before you can call
  * the GetBoundingMetrics() method.
  * Note that for a frame with nested children, the bounding metrics
  * will exactly enclose its children. For example, the bounding metrics
  * of msub is the smallest rectangle that exactly encloses both the 
  * base and the subscript.
  */
  NS_IMETHOD
  GetBoundingMetrics(nsBoundingMetrics& aBoundingMetrics) = 0;

  NS_IMETHOD
  SetBoundingMetrics(const nsBoundingMetrics& aBoundingMetrics) = 0;

  NS_IMETHOD
  GetReference(nsPoint& aReference) = 0;

  NS_IMETHOD
  SetReference(const nsPoint& aReference) = 0;


 /* SUPPORT FOR STRETCHY ELEMENTS */
 /*====================================================================*/

 /* Stretch :
  * Called to ask a stretchy MathML frame to stretch itself depending
  * on its context.
  *
  * An embellished frame is treated in a special way. When it receives a
  * Stretch() command, it passes the command to its embellished child and
  * the stretched size is bubbled up from the inner-most <mo> frame. In other
  * words, the stretch command descend through the embellished hierarchy.
  *
  * @param aStretchDirection [in] the direction where to attempt to
  *        stretch.
  * @param aContainerSize [in] struct that suggests the maximumn size for
  *        the stretched frame. Only member data of the struct that are 
  *        relevant to the direction are used (the rest is ignored). 
  * @param aDesiredStretchSize [in/out] On input the current size
  *        of the frame, on output the size after stretching.
  */
  NS_IMETHOD 
  Stretch(nsIRenderingContext& aRenderingContext,
          nsStretchDirection   aStretchDirection,
          nsBoundingMetrics&   aContainerSize,
          nsHTMLReflowMetrics& aDesiredStretchSize) = 0;

 /* Place :
  * This method is used before returning from Reflow(), or when a MathML frame
  * has just been stretched. It is called to fine-tune the positions of the elements.
  *
  * IMPORTANT: This method uses the origin of child frames (rect.x and rect.y) as
  * placeholders between calls: On invocation, child->GetRect(rect) should give a
  * rect such that rect.x holds the child's descent, rect.y holds the child's ascent, 
  * (rect.width should give the width, and rect.height should give the height).
  * The Place() method will use this information to compute the desired size
  * of the frame.
  *
  * @param aPlaceOrigin [in]
  *        If aPlaceOrigin is false, compute your desired size using the
  *        information in your children's rectangles. However, upon return,
  *        the origins of your children should keep their ascent information, i.e., 
  *        a child rect.x, and rect.y should still act like placeholders for the 
  *        child's descent and ascent. 
  *
  *        If aPlaceOrigin is true, reflow is finished. You should position all
  *        your children, and return your desired size. You should now convert
  *        the origins of your child frames into the coordinate system 
  *        expected by Gecko (which is relative to the upper-left
  *        corner of the parent) and use FinishReflowChild() on your children
  *        to complete post-reflow operations.
  *
  * @param aDesiredSize [out] parameter where you should return your
  *        desired size and your ascent/descent info. Compute your desired size 
  *        using the information in your children's rectangles, and include any
  *        space you want for border/padding in the desired size you return.  
  */
  NS_IMETHOD
  Place(nsIRenderingContext& aRenderingContext,
        PRBool               aPlaceOrigin,
        nsHTMLReflowMetrics& aDesiredSize) = 0;

 /* GetEmbellishData/SetEmbellishData :
  * Get/Set the mEmbellishData member variable.
  */

  NS_IMETHOD
  GetEmbellishData(nsEmbellishData& aEmbellishData) = 0;

  NS_IMETHOD
  SetEmbellishData(const nsEmbellishData& aEmbellishData) = 0;


 /* SUPPORT FOR SCRIPTING ELEMENTS */
 /*====================================================================*/

 /* GetPresentationData/SetPresentationData :
  * Get/Set the mPresentationData member variable.
  */

  NS_IMETHOD
  GetPresentationData(nsPresentationData& aPresentationData) = 0;

  NS_IMETHOD
  SetPresentationData(const nsPresentationData& aPresentationData) = 0;

  /* InheritAutomaticData() / TransmitAutomaticData() :
   * There are precise rules governing each MathML frame and its children.
   * Properties such as the scriptlevel or the embellished nature of a frame
   * depend on those rules. Also, certain properties that we use to emulate
   * TeX rendering rules are frame-dependent too. These two methods are meant
   * to be implemented by frame classes that need to assert specific properties
   * within their subtrees.
   *
   * InheritAutomaticData() is called in a top-down manner [like nsIFrame::Init],
   * as we descend the frame tree, whereas TransmitAutomaticData() is called in a
   * bottom-up manner, as we ascend the tree [like nsIFrame::SetInitialChildList].
   * However, unlike Init() and SetInitialChildList() which are called only once
   * during the life-time of a frame (when initially constructing the frame tree),
   * these two methods are called to build automatic data after the <math>...</math>
   * subtree has been constructed fully, and are called again as we walk a child's
   * subtree to handle dynamic changes that happen in the content model.
   *
   * As a rule of thumb:
   *
   * 1. Use InheritAutomaticData() to set properties related to your ancestors:
   *    - set properties that are intrinsic to yourself
   *    - set properties that depend on the state that you expect your ancestors
   *      to have already reached in their own InheritAutomaticData().
   *    - set properties that your descendants assume that you would have set in
   *      your InheritAutomaticData() -- this way, they can safely query them and
   *      the process will feed upon itself.
   *
   * 2. Use TransmitAutomaticData() to set properties related to your descendants:
   *    - set properties that depend on the state that you expect your descendants
   *      to have reached upon processing their own TransmitAutomaticData().
   *    - transmit properties that your descendants expect that you will transmit to
   *      them in your TransmitAutomaticData() -- this way, they remain up-to-date.
   *    - set properties that your ancestors expect that you would set in your
   *      TransmitAutomaticData() -- this way, they can safely query them and the
   *      process will feed upon itself.
   */

  NS_IMETHOD
  InheritAutomaticData(nsIFrame* aParent) = 0;

  NS_IMETHOD
  TransmitAutomaticData() = 0;

 /* UpdatePresentationData :
  * Increments the scriptlevel of the frame, and updates its displaystyle and
  * compression flags. The displaystyle flag of an environment gets updated
  * according to the MathML specification. A frame becomes "compressed" (or
  * "cramped") according to TeX rendering rules (TeXBook, Ch.17, p.140-141).
  *
  * Note that <mstyle> is the only tag which allows to set
  * <mstyle displaystyle="true|false" scriptlevel="[+|-]number">
  * to reset or increment the scriptlevel in a manual way. 
  * Therefore <mstyle> has its own peculiar version of this method.
  *
  * @param aScriptLevelIncrement [in]
  *        The value with which to increment mScriptLevel in the frame.
  *
  * @param aFlagsValues [in]
  *        The new values (e.g., display, compress) that are going to be
  *        updated.
  *
  * @param aFlagsToUpdate [in]
  *        The flags that are relevant to this call. Since not all calls
  *        are meant to update all flags at once, aFlagsToUpdate is used
  *        to distinguish flags that need to retain their existing values
  *        from flags that need to be turned on (or turned off). If a bit
  *        is set in aFlagsToUpdate, then the corresponding value (which
  *        can be 0 or 1) is taken from aFlagsValues and applied to the
  *        frame. Therefore, by setting their bits in aFlagsToUpdate, and
  *        setting their desired values in aFlagsValues, it is possible to
  *        update some flags in the frame, leaving the other flags unchanged.
  */
  NS_IMETHOD
  UpdatePresentationData(PRInt32         aScriptLevelIncrement,
                         PRUint32        aFlagsValues,
                         PRUint32        aFlagsToUpdate) = 0;

 /* UpdatePresentationDataFromChildAt :
  * Increments the scriplevel and sets the displaystyle and compression flags
  * on the whole tree. For child frames at aFirstIndex up to aLastIndex, this
  * method sets their displaystyle and compression flags, and increment their
  * mScriptLevel with aScriptLevelIncrement. The update is propagated down 
  * the subtrees of each of these child frames. 
  *
  * Note that <mstyle> is the only tag which allows
  * <mstyle displaystyle="true|false" scriptlevel="[+|-]number">
  * to reset or increment the scriptlevel in a manual way.
  * Therefore <mstyle> has its own peculiar version of this method.
  *
  * @param aFirstIndex [in]
  *        Index of the first child from where the update is propagated.
  *
  * @param aLastIndex [in]
  *        Index of the last child where to stop the update.
  *        A value of -1 means up to last existing child.
  *
  * @param aScriptLevelIncrement [in]
  *        The value with which to increment mScriptLevel in the whole sub-trees.
  *
  * @param aFlagsValues [in]
  *        The new values (e.g., display, compress) that are going to be
  *        assigned in the whole sub-trees.
  *
  * @param aFlagsToUpdate [in]
  *        The flags that are relevant to this call. See UpdatePresentationData()
  *        for more details about this parameter.
  */
  NS_IMETHOD
  UpdatePresentationDataFromChildAt(PRInt32         aFirstIndex,
                                    PRInt32         aLastIndex,
                                    PRInt32         aScriptLevelIncrement,
                                    PRUint32        aFlagsValues,
                                    PRUint32        aFlagsToUpdate) = 0;

 /* ReResolveScriptStyle :
  * During frame construction, the Style System gives us style contexts in
  * which the sizes of the fonts are not suitable for scripting elements.
  * Our expected behavior is that, when given the markup <mtag>base arguments</mtag>,
  * we want to render the 'base' in a normal size, and the 'arguments' in a smaller
  * size. This is a common functionality to tags like msub, msup, msubsup, mover,
  * munder, munderover, mmultiscripts. Moreover, we want the reduction of the font
  * size to happen in a top-down manner within the hierarchies of sub-expressions;
  * and more importantly, we don't want the sizes to keep decreasing up to a point
  * where the scripts become unreadably small.
  *
  * In general, this scaling effet arises when the scriptlevel changes between a
  * parent and a child. Whenever the scriptlevel changes, either automatically or
  * by being explicitly incremented, decremented, or set, the current font size has
  * to be multiplied by the predefined value of 'scriptsizemultiplier' to the power
  * of the change in the scriptlevel, and this scaling effect (downwards or upwards)
  * has to be propagated down the subtrees, with the caveat that the font size is
  * never allowed to go below the predefined value of 'scriptminsize' within a 
  * sub-expression.
  *
  * ReResolveScriptStyle() will walk a subtree to cause this mathml-specific behavior
  * to happen. The method is recursive and only a top-level parent wishing to reflect
  * the changes in its children needs to call to the method.
  *
  * This function is *very* expensive. Unfortunately, there isn't much
  * to do about it at the moment. For background on the problem @see 
  * http://groups.google.com/groups?selm=3A9192B5.D22B6C38%40maths.uq.edu.au
  */
  NS_IMETHOD
  ReResolveScriptStyle(PRInt32 aParentScriptLevel) = 0;
};

// struct used by a container frame to keep track of its embellishments.
// By convention, the data that we keep here is bubbled from the embellished
// hierarchy, and it remains unchanged unless we have to recover from a change
// that occurs in the embellished hierarchy. The struct remains in its nil
// state in those frames that are not part of the embellished hierarchy.
struct nsEmbellishData {
  // bits used to mark certain properties of our embellishments 
  PRUint32 flags;

  // pointer on the <mo> frame at the core of the embellished hierarchy
  nsIFrame* coreFrame;

  // stretchy direction that the nsMathMLChar owned by the core <mo> supports
  nsStretchDirection direction;

  // spacing that may come from <mo> depending on its 'form'. Since
  // the 'form' may also depend on the position of the outermost
  // embellished ancestor, the set up of these values may require
  // looking up the position of our ancestors.
  nscoord leftSpace;
  nscoord rightSpace;

  nsEmbellishData() {
    flags = 0;
    coreFrame = nsnull;
    direction = NS_STRETCH_DIRECTION_UNSUPPORTED;
    leftSpace = 0;
    rightSpace = 0;
  }
};

// struct used by a container frame to modulate its presentation.
// By convention, the data that we keep in this struct can change depending
// on any of our ancestors and/or descendants. If a data can be resolved
// solely from the embellished hierarchy, and it remains immutable once
// resolved, we put it in |nsEmbellishData|. If it can be affected by other
// things, it comes here. This struct is updated as we receive information
// transmitted by our ancestors and is kept in sync with changes in our
// descendants that affects us.
struct nsPresentationData {
  // bits for: displaystyle, compressed, etc
  PRUint32 flags;

  // handy pointer on our base child (the 'nucleus' in TeX), but it may be
  // null here (e.g., tags like <mrow>, <mfrac>, <mtable>, etc, won't
  // pick a particular child in their child list to be the base)
  nsIFrame* baseFrame;

  // up-pointer on the mstyle frame, if any, that defines the scope
  nsIFrame* mstyle;

  // level of nested frames within: msub, msup, msubsup, munder,
  // mover, munderover, mmultiscripts, mfrac, mroot, mtable.
  PRInt32 scriptLevel;

  nsPresentationData() {
    flags = 0;
    baseFrame = nsnull;
    mstyle = nsnull;
    scriptLevel = 0;
  }
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsIMathMLFrame, NS_IMATHMLFRAME_IID)


// ==========================================================================
// Bits used for the presentation flags -- these bits are set
// in their relevant situation as they become available

// This bit is set if the frame is in the *context* of displaystyle=true.
// Note: This doesn't mean that the frame has displaystyle=true as attribute,
// <mstyle> is the only tag which allows <mstyle displaystyle="true|false">.
// The bit merely tells the context of the frame. In the context of 
// displaystyle="false", it is intended to slightly alter how the
// rendering is done in inline mode.
#define NS_MATHML_DISPLAYSTYLE                        0x00000001

// This bit is used to emulate TeX rendering. 
// Internal use only, cannot be set by the user with an attribute.
#define NS_MATHML_COMPRESSED                          0x00000002

// This bit is set if the frame will fire a vertical stretch
// command on all its (non-empty) children.
// Tags like <mrow> (or an inferred mrow), mpadded, etc, will fire a
// vertical stretch command on all their non-empty children
#define NS_MATHML_STRETCH_ALL_CHILDREN_VERTICALLY     0x00000004

// This bit is set if the frame will fire a horizontal stretch
// command on all its (non-empty) children.
// Tags like munder, mover, munderover, will fire a 
// horizontal stretch command on all their non-empty children
#define NS_MATHML_STRETCH_ALL_CHILDREN_HORIZONTALLY   0x00000008

// This bit is set if the frame is actually an <mstyle> frame *and* that
// <mstyle> frame has an explicit attribute scriptlevel="value".
// Note: the flag is not set if the <mstyle> instead has an incremental +/-value.
#define NS_MATHML_MSTYLE_WITH_EXPLICIT_SCRIPTLEVEL    0x00000010

// This bit is set if the frame is actually an <mstyle> *and* that
// <mstyle> has an explicit attribute displaystyle="true" or "false"
#define NS_MATHML_MSTYLE_WITH_DISPLAYSTYLE            0x00000020

// This bit is set when the frame cannot be formatted due to an
// error (e.g., invalid markup such as a <msup> without an overscript).
// When set, a visual feedback will be provided to the user.
#define NS_MATHML_ERROR                               0x80000000

// a bit used for debug
#define NS_MATHML_STRETCH_DONE                        0x20000000

// This bit is used for visual debug. When set, the bounding box
// of your frame is painted. This visual debug enable to ensure that
// you have properly filled your mReference and mBoundingMetrics in
// Place().
#define NS_MATHML_SHOW_BOUNDING_METRICS               0x10000000

// Macros that retrieve those bits

#define NS_MATHML_IS_DISPLAYSTYLE(_flags) \
  (NS_MATHML_DISPLAYSTYLE == ((_flags) & NS_MATHML_DISPLAYSTYLE))

#define NS_MATHML_IS_COMPRESSED(_flags) \
  (NS_MATHML_COMPRESSED == ((_flags) & NS_MATHML_COMPRESSED))

#define NS_MATHML_WILL_STRETCH_ALL_CHILDREN_VERTICALLY(_flags) \
  (NS_MATHML_STRETCH_ALL_CHILDREN_VERTICALLY == ((_flags) & NS_MATHML_STRETCH_ALL_CHILDREN_VERTICALLY))

#define NS_MATHML_WILL_STRETCH_ALL_CHILDREN_HORIZONTALLY(_flags) \
  (NS_MATHML_STRETCH_ALL_CHILDREN_HORIZONTALLY == ((_flags) & NS_MATHML_STRETCH_ALL_CHILDREN_HORIZONTALLY))

#define NS_MATHML_IS_MSTYLE_WITH_DISPLAYSTYLE(_flags) \
  (NS_MATHML_MSTYLE_WITH_DISPLAYSTYLE == ((_flags) & NS_MATHML_MSTYLE_WITH_DISPLAYSTYLE))

#define NS_MATHML_IS_MSTYLE_WITH_EXPLICIT_SCRIPTLEVEL(_flags) \
  (NS_MATHML_MSTYLE_WITH_EXPLICIT_SCRIPTLEVEL == ((_flags) & NS_MATHML_MSTYLE_WITH_EXPLICIT_SCRIPTLEVEL))

#define NS_MATHML_HAS_ERROR(_flags) \
  (NS_MATHML_ERROR == ((_flags) & NS_MATHML_ERROR))

#define NS_MATHML_STRETCH_WAS_DONE(_flags) \
  (NS_MATHML_STRETCH_DONE == ((_flags) & NS_MATHML_STRETCH_DONE))

#define NS_MATHML_PAINT_BOUNDING_METRICS(_flags) \
  (NS_MATHML_SHOW_BOUNDING_METRICS == ((_flags) & NS_MATHML_SHOW_BOUNDING_METRICS))

// ==========================================================================
// Bits used for the embellish flags -- these bits are set
// in their relevant situation as they become available

// This bit is set if the frame is an embellished operator. 
#define NS_MATHML_EMBELLISH_OPERATOR                0x00000001

// This bit is set if the frame is an <mo> frame or an embellihsed
// operator for which the core <mo> has movablelimits="true"
#define NS_MATHML_EMBELLISH_MOVABLELIMITS           0x00000002

// This bit is set if the frame is an <mo> frame or an embellihsed
// operator for which the core <mo> has accent="true"
#define NS_MATHML_EMBELLISH_ACCENT                  0x00000004

// This bit is set if the frame is an <mover> or <munderover> with
// an accent frame
#define NS_MATHML_EMBELLISH_ACCENTOVER              0x00000008

// This bit is set if the frame is an <munder> or <munderover> with
// an accentunder frame
#define NS_MATHML_EMBELLISH_ACCENTUNDER             0x00000010

// Macros that retrieve those bits

#define NS_MATHML_IS_EMBELLISH_OPERATOR(_flags) \
  (NS_MATHML_EMBELLISH_OPERATOR == ((_flags) & NS_MATHML_EMBELLISH_OPERATOR))

#define NS_MATHML_EMBELLISH_IS_MOVABLELIMITS(_flags) \
  (NS_MATHML_EMBELLISH_MOVABLELIMITS == ((_flags) & NS_MATHML_EMBELLISH_MOVABLELIMITS))

#define NS_MATHML_EMBELLISH_IS_ACCENT(_flags) \
  (NS_MATHML_EMBELLISH_ACCENT == ((_flags) & NS_MATHML_EMBELLISH_ACCENT))

#define NS_MATHML_EMBELLISH_IS_ACCENTOVER(_flags) \
  (NS_MATHML_EMBELLISH_ACCENTOVER == ((_flags) & NS_MATHML_EMBELLISH_ACCENTOVER))

#define NS_MATHML_EMBELLISH_IS_ACCENTUNDER(_flags) \
  (NS_MATHML_EMBELLISH_ACCENTUNDER == ((_flags) & NS_MATHML_EMBELLISH_ACCENTUNDER))

#endif /* nsIMathMLFrame_h___ */
