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

#ifndef nsRegionXlib_h___
#define nsRegionXlib_h___

#include "nsIRegion.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

class nsRegionXlib : public nsIRegion
{
 public:
  nsRegionXlib();
  virtual ~nsRegionXlib();

  NS_DECL_ISUPPORTS

  nsresult Init();

  void SetTo(const nsIRegion &aRegion);
  void SetTo(PRInt32 aX, PRInt32 aY, PRInt32 aWidth, PRInt32 aHeight);
  void Intersect(const nsIRegion &aRegion);
  void Intersect(PRInt32 aX, PRInt32 aY, PRInt32 aWidth, PRInt32 aHeight);
  void Union(const nsIRegion &aRegion);
  void Union(PRInt32 aX, PRInt32 aY, PRInt32 aWidth, PRInt32 aHeight);
  void Subtract(const nsIRegion &aRegion);
  void Subtract(PRInt32 aX, PRInt32 aY, PRInt32 aWidth, PRInt32 aHeight);
  PRBool IsEmpty(void);
  PRBool IsEqual(const nsIRegion &aRegion);
  void GetBoundingBox(PRInt32 *aX, PRInt32 *aY, PRInt32 *aWidth, PRInt32 *aHeight);
  void Offset(PRInt32 aXOffset, PRInt32 aYOffset);
  PRBool ContainsRect(PRInt32 aX, PRInt32 aY, PRInt32 aWidth, PRInt32 aHeight);
  NS_IMETHOD GetRects(nsRegionRectSet **aRects);
  NS_IMETHOD FreeRects(nsRegionRectSet *aRects);
  NS_IMETHOD GetNativeRegion(void *&aRegion) const;
  NS_IMETHOD GetRegionComplexity(nsRegionComplexity &aComplexity) const;
  NS_IMETHOD GetNumRects(PRUint32 *aRects) const;

protected:
  Region xlib_region_copy(Region region);
  Region xlib_region_from_rect(PRInt32 aX, PRInt32 aY, PRInt32 aWidth, PRInt32 aHeight);

private:
  Region mRegion;
  static Region copyRegion;

  inline Region GetCopyRegion();
  inline void SetRegionEmpty();
};

#endif
