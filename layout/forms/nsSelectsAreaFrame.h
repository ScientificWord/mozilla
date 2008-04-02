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
#ifndef nsSelectsAreaFrame_h___
#define nsSelectsAreaFrame_h___

#include "nsAreaFrame.h"
class nsIContent;

/**
 * The area frame has an additional named child list:
 * - "Absolute-list" which contains the absolutely positioned frames
 *
 * @see nsGkAtoms::absoluteList
 */
class nsSelectsAreaFrame : public nsAreaFrame
{
public:
  friend nsIFrame* NS_NewSelectsAreaFrame(nsIPresShell* aShell, nsStyleContext* aContext, PRUint32 aFlags);

  NS_IMETHOD BuildDisplayList(nsDisplayListBuilder*   aBuilder,
                              const nsRect&           aDirtyRect,
                              const nsDisplayListSet& aLists);

  nsresult BuildDisplayListInternal(nsDisplayListBuilder*   aBuilder,
                                    const nsRect&           aDirtyRect,
                                    const nsDisplayListSet& aLists);

  NS_IMETHOD Reflow(nsPresContext*          aCX,
                    nsHTMLReflowMetrics&     aDesiredSize,
                    const nsHTMLReflowState& aReflowState,
                    nsReflowStatus&          aStatus);

  static PRBool IsOptionElement(nsIContent* aContent);
  static PRBool IsOptionElementFrame(nsIFrame *aFrame);
  
  nscoord HeightOfARow() const { return mHeightOfARow; }
  
protected:
  nsSelectsAreaFrame(nsStyleContext* aContext) :
    nsAreaFrame(aContext),
    mHeightOfARow(0)
  {}

  // We cache the height of a single row so that changes to the "size"
  // attribute, padding, etc. can all be handled with only one reflow.  We'll
  // have to reflow twice if someone changes our font size or something like
  // that, so that the heights of our options will change.
  nscoord mHeightOfARow;
};

#endif /* nsSelectsAreaFrame_h___ */
