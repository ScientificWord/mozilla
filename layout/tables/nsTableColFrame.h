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
#ifndef nsTableColFrame_h__
#define nsTableColFrame_h__

#include "nscore.h"
#include "nsContainerFrame.h"
#include "nsTablePainter.h"

class nsVoidArray;
class nsTableCellFrame;

// this is used to index arrays of widths in nsColFrame and to group important widths
// for calculations. It is important that the order: min, desired, fixed be maintained
// for each category (con, adj).
#define WIDTH_NOT_SET   -1
#define NUM_WIDTHS      10
#define NUM_MAJOR_WIDTHS 3 // MIN, DES, FIX
#define MIN_CON          0 // minimum width required of the content + padding
#define DES_CON          1 // desired width of the content + padding
#define FIX              2 // fixed width either from the content or cell, col, etc. + padding
#define MIN_ADJ          3 // minimum width + padding due to col spans
#define DES_ADJ          4 // desired width + padding due to col spans
#define FIX_ADJ          5 // fixed width + padding due to col spans
#define PCT              6 // percent width of cell or col 
#define PCT_ADJ          7 // percent width of cell or col from percent colspan
#define MIN_PRO          8 // desired width due to proportional <col>s or cols attribute
#define FINAL            9 // width after the table has been balanced, considering all of the others

enum nsColConstraint {
  eNoConstraint          = 0,
  ePixelConstraint       = 1,      // pixel width 
  ePercentConstraint     = 2,      // percent width
  eProportionConstraint  = 3,      // 1*, 2*, etc. cols attribute assigns 1*
  e0ProportionConstraint = 4       // 0*, means to force to min width
};

enum nsTableColType {
  eColContent            = 0, // there is real col content associated   
  eColAnonymousCol       = 1, // the result of a span on a col
  eColAnonymousColGroup  = 2, // the result of a span on a col group
  eColAnonymousCell      = 3  // the result of a cell alone
};

class nsTableColFrame : public nsFrame {
public:

  enum {eWIDTH_SOURCE_NONE          =0,   // no cell has contributed to the width style
        eWIDTH_SOURCE_CELL          =1,   // a cell specified a width
        eWIDTH_SOURCE_CELL_WITH_SPAN=2    // a cell implicitly specified a width via colspan
  };

  nsTableColType GetColType() const;
  void SetColType(nsTableColType aType);

  /** instantiate a new instance of nsTableRowFrame.
    * @param aPresShell the pres shell for this frame
    *
    * @return           the frame that was created
    */
  friend nsIFrame* NS_NewTableColFrame(nsIPresShell* aPresShell, nsStyleContext*  aContext);

  nsStyleCoord GetStyleWidth() const;

  PRInt32 GetColIndex() const;
  
  void SetColIndex (PRInt32 aColIndex);

  nsTableColFrame* GetNextCol() const;

  NS_IMETHOD Init(nsIContent*      aContent,
                  nsIFrame*        aParent,
                  nsIFrame*        aPrevInFlow);

  NS_IMETHOD Reflow(nsPresContext*          aPresContext,
                    nsHTMLReflowMetrics&     aDesiredSize,
                    const nsHTMLReflowState& aReflowState,
                    nsReflowStatus&          aStatus);

  /**
   * Table columns never paint anything, nor receive events.
   */
  NS_IMETHOD BuildDisplayList(nsDisplayListBuilder*   aBuilder,
                              const nsRect&           aDirtyRect,
                              const nsDisplayListSet& aLists) { return NS_OK; }

  /**
   * Get the "type" of the frame
   *
   * @see nsLayoutAtoms::tableColFrame
   */
  virtual nsIAtom* GetType() const;
  
#ifdef DEBUG
  NS_IMETHOD GetFrameName(nsAString& aResult) const;
#endif

  /** return the number of the columns the col represents.  always >= 0 */
  virtual PRInt32 GetSpan ();

  /** convenience method, calls into cellmap */
  nsVoidArray * GetCells();

  nscoord GetWidth(PRUint32 aWidthType);
  void    SetWidth(PRUint32 aWidthType,
                   nscoord  aWidth);
  nscoord GetMinWidth();
  nscoord GetDesWidth();
  nscoord GetFixWidth();
  nscoord GetPctWidth();

  void            SetConstraint(nsColConstraint aConstraint);
  nsColConstraint GetConstraint() const;

  /** convenience method, calls into cellmap */
  PRInt32 Count() const;

  void ResetSizingInfo();

  nscoord GetLeftBorderWidth(float* aPixelsToTwips = nsnull);
  void    SetLeftBorderWidth(BCPixelSize aWidth);
  nscoord GetRightBorderWidth(float* aPixelsToTwips = nsnull);
  void    SetRightBorderWidth(BCPixelSize aWidth);

  /**
   * Gets inner border widths before collapsing with cell borders
   * Caller must get left border from previous column or from table
   * GetContinuousBCBorderWidth will not overwrite aBorder.left
   * see nsTablePainter about continuous borders
   *
   * @return outer right border width (left inner for next column)
   */
  nscoord GetContinuousBCBorderWidth(float     aPixelsToTwips,
                                     nsMargin& aBorder);
  /**
   * Set full border widths before collapsing with cell borders
   * @param aForSide - side to set; only valid for top, right, and bottom
   */
  void SetContinuousBCBorderWidth(PRUint8     aForSide,
                                  BCPixelSize aPixelValue);
#ifdef DEBUG
  void Dump(PRInt32 aIndent);
#endif

protected:

  nsTableColFrame(nsStyleContext* aContext);
  ~nsTableColFrame();

  // the index of the column with respect to the whole tabble (starting at 0) 
  // it should never be smaller then the start column index of the parent 
  // colgroup
  PRUint32 mColIndex:        16;
  
  // border width in pixels of the inner half of the border only
  BCPixelSize mLeftBorderWidth;
  BCPixelSize mRightBorderWidth;
  BCPixelSize mTopContBorderWidth;
  BCPixelSize mRightContBorderWidth;
  BCPixelSize mBottomContBorderWidth;
  // Widths including MIN_CON, DES_CON, FIX_CON, MIN_ADJ, DES_ADJ, FIX_ADJ, PCT, PCT_ADJ, MIN_PRO, FINAL
  // Widths including MIN_CON, DES_CON, FIX_CON, MIN_ADJ, DES_ADJ, FIX_ADJ, PCT, PCT_ADJ, MIN_PRO, FINAL
  // XXX these could be stored as pixels and converted to twips for a savings of 10 x 2 bytes.
  nscoord           mWidths[NUM_WIDTHS];
};

inline PRInt32 nsTableColFrame::GetColIndex() const
{
  return mColIndex; 
}

inline void nsTableColFrame::SetColIndex (PRInt32 aColIndex)
{ 
  mColIndex = aColIndex; 
}

inline nscoord nsTableColFrame::GetLeftBorderWidth(float*  aPixelsToTwips)
{
  nscoord width = (aPixelsToTwips) ? NSToCoordRound(*aPixelsToTwips * mLeftBorderWidth) : mLeftBorderWidth;
  return width;
}

inline void nsTableColFrame::SetLeftBorderWidth(BCPixelSize aWidth)
{
  mLeftBorderWidth = aWidth;
}

inline nscoord nsTableColFrame::GetRightBorderWidth(float*  aPixelsToTwips)
{
  nscoord width = (aPixelsToTwips) ? NSToCoordRound(*aPixelsToTwips * mRightBorderWidth) : mRightBorderWidth;
  return width;
}

inline void nsTableColFrame::SetRightBorderWidth(BCPixelSize aWidth)
{
  mRightBorderWidth = aWidth;
}

inline nscoord
nsTableColFrame::GetContinuousBCBorderWidth(float     aPixelsToTwips,
                                            nsMargin& aBorder)
{
  aBorder.top = BC_BORDER_BOTTOM_HALF_COORD(aPixelsToTwips,
                                            mTopContBorderWidth);
  aBorder.right = BC_BORDER_LEFT_HALF_COORD(aPixelsToTwips,
                                            mRightContBorderWidth);
  aBorder.bottom = BC_BORDER_TOP_HALF_COORD(aPixelsToTwips,
                                            mBottomContBorderWidth);
  return BC_BORDER_RIGHT_HALF_COORD(aPixelsToTwips, mRightContBorderWidth);
}

#endif

