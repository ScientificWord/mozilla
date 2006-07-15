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

#ifndef nsDrawingSurfaceMac_h___
#define nsDrawingSurfaceMac_h___

#include "nsIDrawingSurface.h"
#include "nsIDrawingSurfaceMac.h"

#include "nsGfxUtils.h"
#include "nsCarbonHelpers.h"

class nsGraphicState;

class nsDrawingSurfaceMac : public nsIDrawingSurface,
                            nsIDrawingSurfaceMac
{
public:
  nsDrawingSurfaceMac();
  virtual ~nsDrawingSurfaceMac();

  NS_DECL_ISUPPORTS

  //nsIDrawingSurface interface
  NS_IMETHOD Lock(PRInt32 aX, PRInt32 aY, PRUint32 aWidth, PRUint32 aHeight,
                  void **aBits, PRInt32 *aStride, PRInt32 *aWidthBytes,
                  PRUint32 aFlags);
  NS_IMETHOD Unlock(void);
  NS_IMETHOD GetDimensions(PRUint32 *aWidth, PRUint32 *aHeight);
  NS_IMETHOD IsOffscreen(PRBool *aOffScreen) { *aOffScreen = mIsOffscreen; return NS_OK; }
  NS_IMETHOD IsPixelAddressable(PRBool *aAddressable);
  NS_IMETHOD GetPixelFormat(nsPixelFormat *aFormat);

  //nsIDrawingSurfaceMac interface
  NS_IMETHOD Init(nsIDrawingSurface* aDS);
  NS_IMETHOD Init(CGrafPtr aThePort);
  NS_IMETHOD Init(nsIWidget *aTheWidget);
  NS_IMETHOD Init(PRUint32 aDepth,PRUint32 aWidth, PRUint32 aHeight,PRUint32 aFlags);
	NS_IMETHOD GetGrafPtr(CGrafPtr	*aTheGrafPtr) { *aTheGrafPtr = mPort; return NS_OK; }
  NS_IMETHOD_(CGContextRef) StartQuartzDrawing();
  NS_IMETHOD_(void) EndQuartzDrawing(CGContextRef aContext);

  // locals
	nsGraphicState*	GetGS(void) {return mGS;}

private:
  CGrafPtr      mPort;      // the onscreen or offscreen CGrafPtr;	

  PRUint32      mWidth;
  PRUint32      mHeight;
  PRInt32       mLockOffset;
  PRInt32       mLockHeight;
  PRUint32      mLockFlags;
  PRBool        mIsOffscreen;
  PRBool        mIsLocked;

  nsGraphicState* mGS;      // a graphics state for the surface
#ifdef MOZ_WIDGET_COCOA
  void* mWidgetView;        // non-retained ('weak') ref to NSView
#endif
};

#endif
