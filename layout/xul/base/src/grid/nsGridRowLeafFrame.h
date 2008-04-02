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

/**

  Eric D Vaughan
  A frame that can have multiple children. Only one child may be displayed at one time. So the
  can be flipped though like a deck of cards.
 
**/

#ifndef nsGridRowLeafFrame_h___
#define nsGridRowLeafFrame_h___

#include "nsBoxFrame.h"

/**
 * A frame representing a grid row (or column).  Grid row (and column)
 * elements are the children of row group (or column group) elements,
 * and their children are placed one to a cell.
 */
// XXXldb This needs a better name that indicates that it's for any grid
// row.
class nsGridRowLeafFrame : public nsBoxFrame
{
public:

  friend nsIFrame* NS_NewGridRowLeafFrame(nsIPresShell* aPresShell,
                                          nsStyleContext* aContext,
                                          nsIBoxLayout* aLayoutManager);

#ifdef NS_DEBUG
  NS_IMETHOD GetFrameName(nsAString& aResult) const
  {
      return MakeFrameName(NS_LITERAL_STRING("nsGridRowLeaf"), aResult);
  }
#endif

  nsGridRowLeafFrame(nsIPresShell* aPresShell,
                     nsStyleContext* aContext,
                     PRBool aIsRoot,
                     nsIBoxLayout* aLayoutManager):
    nsBoxFrame(aPresShell, aContext, aIsRoot, aLayoutManager) {}

  NS_IMETHOD GetBorderAndPadding(nsMargin& aBorderAndPadding);

}; // class nsGridRowLeafFrame



#endif

