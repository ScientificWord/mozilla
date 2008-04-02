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

/* code for management of floats that implements space manager interfaces */

#include "nsCOMPtr.h"
#include "nsBlockBandData.h"
#include "nsIFrame.h"
#include "nsHTMLReflowState.h"
#include "nsPresContext.h"
#include "nsIPresShell.h"

nsBlockBandData::nsBlockBandData()
  : mSpaceManager(nsnull),
    mSpaceManagerX(0),
    mSpaceManagerY(0),
    mSpace(0, 0)
{
  mSize = NS_BLOCK_BAND_DATA_TRAPS;
  mTrapezoids = mData;
}

nsBlockBandData::~nsBlockBandData()
{
  if (mTrapezoids != mData) {
    delete [] mTrapezoids;
  }
}

nsresult
nsBlockBandData::Init(nsSpaceManager* aSpaceManager,
                      const nsSize& aSpace)
{
  NS_PRECONDITION(aSpaceManager, "null pointer");

  mSpaceManager = aSpaceManager;
  aSpaceManager->GetTranslation(mSpaceManagerX, mSpaceManagerY);

  mSpace = aSpace;
  mLeftFloats = 0;
  mRightFloats = 0;
  return NS_OK;
}

// Get the available reflow space for the current y coordinate. The
// available space is relative to our coordinate system (0,0) is our
// upper left corner.
nsresult
nsBlockBandData::GetAvailableSpace(nscoord aY, PRBool aRelaxHeightConstraint,
                                   nsRect& aResult)
{
  // Get the raw band data for the given Y coordinate
  nsresult rv = GetBandData(aY, aRelaxHeightConstraint);
  if (NS_FAILED(rv)) { return rv; }

  // Compute the bounding rect of the available space, i.e. space
  // between any left and right floats.
  ComputeAvailSpaceRect();
  aResult = mAvailSpace;
#ifdef REALLY_NOISY_COMPUTEAVAILSPACERECT
  printf("nsBBD %p GetAvailableSpace(%d) returning (%d, %d, %d, %d)\n",
          this, aY, aResult.x, aResult.y, aResult.width, aResult.height);
#endif
  return NS_OK;
}

// the code below should never loop more than a very few times.
// this is a safety valve to see if we've gone off the deep end
#define ERROR_TOO_MANY_ITERATIONS 1000

/* nsBlockBandData methods should never call mSpaceManager->GetBandData directly.
 * They should always call nsBlockBandData::GetBandData() instead.
 */
nsresult
nsBlockBandData::GetBandData(nscoord aY, PRBool aRelaxHeightConstraint)
{
  NS_ASSERTION(mSpaceManager, "bad state, no space manager");
  PRInt32 iterations =0;
  nsSize space = mSpace;
  if (aRelaxHeightConstraint) {
    space.height = NS_UNCONSTRAINEDSIZE;
  }
  nsresult rv = mSpaceManager->GetBandData(aY, space, *this);
  while (NS_FAILED(rv)) {
    iterations++;
    if (iterations>ERROR_TOO_MANY_ITERATIONS)
    {
      NS_ASSERTION(PR_FALSE, "too many iterations in nsBlockBandData::GetBandData");
      return NS_ERROR_FAILURE;
    }
    // We need more space for our bands
    NS_ASSERTION(mTrapezoids, "bad state, no mTrapezoids");
    if (mTrapezoids && (mTrapezoids != mData)) {
      delete [] mTrapezoids;
    }
    PRInt32 newSize = mSize * 2;
    if (newSize<mCount) {
      newSize = mCount;
    }
    mTrapezoids = new nsBandTrapezoid[newSize];
    NS_POSTCONDITION(mTrapezoids, "failure allocating mTrapezoids");
    if (!mTrapezoids) {
      return NS_ERROR_OUT_OF_MEMORY;
    }
    mSize = newSize;
    rv = mSpaceManager->GetBandData(aY, space, *this);
  }
  NS_POSTCONDITION(mCount<=mSize, "bad state, count > size");
  return NS_OK;
}



/**
 * Computes the bounding rect of the available space, i.e. space
 * between any left and right floats. Uses the current trapezoid
 * data, see nsISpaceManager::GetBandData(). Also updates member
 * data "availSpace".
 */
void
nsBlockBandData::ComputeAvailSpaceRect()
{
#ifdef REALLY_NOISY_COMPUTEAVAILSPACERECT
  printf("nsBlockBandData::ComputeAvailSpaceRect %p with count %d\n", this, mCount);
#endif
  if (0 == mCount) {
    mAvailSpace.x = 0;
    mAvailSpace.y = 0;
    mAvailSpace.width = 0;
    mAvailSpace.height = 0;
    mLeftFloats = 0;
    mRightFloats = 0;
    return;
  }

  nsBandTrapezoid* trapezoid = mTrapezoids;
  // The trapezoid to the left of the first right-floated trapezoid.
  nsBandTrapezoid* rightTrapezoid = nsnull;

  PRInt32 leftFloats = 0;
  PRInt32 rightFloats = 0;
  if (mCount > 1) {
    // If there's more than one trapezoid that means there are floats
    PRInt32 i;

    // Examine each trapezoid in the band, counting up the number of
    // left and right floats. Use the right-most float to
    // determine where the right edge of the available space is.
    NS_PRECONDITION(mCount<=mSize, "bad state, count > size");
    for (i = 0; i < mCount; i++) {
      trapezoid = &mTrapezoids[i];
      if (trapezoid->mFrames) {
#ifdef REALLY_NOISY_COMPUTEAVAILSPACERECT
        printf("band %p checking !Avail trap %p with frame %p\n", this, trapezoid, trapezoid->mFrames);
#endif
        const nsSmallVoidArray* frames = trapezoid->mFrames;
        const PRInt32 numFrames = frames->Count();
        NS_ASSERTION(numFrames > 0, "bad trapezoid frame list");
        for (PRInt32 j = 0; j < numFrames; j++) {
          nsIFrame* f = static_cast<nsIFrame*>(frames->ElementAt(j));
          const nsStyleDisplay* display = f->GetStyleDisplay();
          if (NS_STYLE_FLOAT_LEFT == display->mFloats) {
            leftFloats++;
          }
          else if (NS_STYLE_FLOAT_RIGHT == display->mFloats) {
            rightFloats++;
            if ((nsnull == rightTrapezoid) && (i > 0)) {
              rightTrapezoid = &mTrapezoids[i - 1];
            }
          }
        }
      }
    }
  }
  else if (mTrapezoids[0].mFrames) {
    // We have a float using up all the available space
    leftFloats = 1;
  }
#ifdef REALLY_NOISY_COMPUTEAVAILSPACERECT
  printf("band %p has floats %d, %d\n", this, leftFloats, rightFloats);
#endif
  mLeftFloats = leftFloats;
  mRightFloats = rightFloats;

  // We look for available space in the last trapezoid before the
  // first right float, or in the last trapezoid if there is no right
  // float or no trapezoid before the first right float.
  if (nsnull != rightTrapezoid) {
    trapezoid = rightTrapezoid;
  }
  trapezoid->GetRect(mAvailSpace);

  // When there is no available space, we still need a proper X
  // coordinate to place objects that end up here anyway.
  const nsSmallVoidArray* frames = trapezoid->mFrames;
  if (frames) {
    // It's not clear what coordinate to use when there is no
    // available space and the space is multiply occupied...So: If
    // any of the floats that are a part of the trapezoid are left
    // floats then we move over to the right edge of the
    // unavaliable space.
    const PRInt32 numFrames = frames->Count();
    NS_ASSERTION(numFrames > 0, "bad trapezoid frame list");
    for (PRInt32 j = 0; j < numFrames; j++) {
      nsIFrame* f = static_cast<nsIFrame*>(frames->ElementAt(j));
      const nsStyleDisplay* display = f->GetStyleDisplay();
      if (NS_STYLE_FLOAT_LEFT == display->mFloats) {
        mAvailSpace.x = mAvailSpace.XMost();
        break;
      }
    }
    mAvailSpace.width = 0;
  }

  // Fixup width
  if (NS_UNCONSTRAINEDSIZE == mSpace.width) {
    mAvailSpace.width = NS_UNCONSTRAINEDSIZE;
  }
#ifdef REALLY_NOISY_COMPUTEAVAILSPACERECT
  printf("  ComputeAvailSpaceRect settting state mAvailSpace (%d,%d,%d,%d)\n", 
         mAvailSpace.x, mAvailSpace.y, mAvailSpace.width, mAvailSpace.height);
#endif

}

#ifdef DEBUG
void nsBlockBandData::List()
{
  printf("nsBlockBandData %p sm=%p, sm coord = (%d,%d), mSpace = (%d,%d)\n",
          this, mSpaceManager, mSpaceManagerX, mSpaceManagerY,
          mSpace.width, mSpace.height);
  printf("  availSpace=(%d, %d, %d, %d), floats l=%d r=%d\n",
          mAvailSpace.x, mAvailSpace.y, mAvailSpace.width, mAvailSpace.height,
          mLeftFloats, mRightFloats);
}
#endif
