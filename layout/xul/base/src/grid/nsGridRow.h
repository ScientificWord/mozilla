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
 
  Author:
  Eric D Vaughan

**/

#ifndef nsGridRow_h___
#define nsGridRow_h___

#include "nsIFrame.h"

class nsGridLayout2;
class nsBoxLayoutState;

/**
 * The row (or column) data structure in the grid cellmap.
 */
class nsGridRow
{
public:
   nsGridRow();
   ~nsGridRow();
   
   void Init(nsIBox* aBox, PRBool aIsBogus);
   void MarkDirty(nsBoxLayoutState& aState);

// accessors
   nsIBox* GetBox()   { return mBox;          }
   PRBool IsPrefSet() { return (mPref != -1); }
   PRBool IsMinSet()  { return (mMin  != -1); }
   PRBool IsMaxSet()  { return (mMax  != -1); } 
   PRBool IsFlexSet() { return (mFlex != -1); }
   PRBool IsOffsetSet() { return (mTop != -1 && mBottom != -1); }
   PRBool IsCollapsed(nsBoxLayoutState& aState);

public:

   PRBool  mIsBogus;
   nsIBox* mBox;
   nscoord mFlex;
   nscoord mPref;
   nscoord mMin;
   nscoord mMax;
   nscoord mTop;
   nscoord mBottom;
   nscoord mTopMargin;
   nscoord mBottomMargin;

};


#endif

