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

#ifndef nsIDrawingSurfaceWin_h___
#define nsIDrawingSurfaceWin_h___

#include "nsIDrawingSurface.h"
#include <windows.h>

// windows specific drawing surface method set

#define NS_IDRAWING_SURFACE_WIN_IID   \
{ 0x1ed958b0, 0xcab6, 0x11d2, \
{ 0xa8, 0x49, 0x00, 0x40, 0x95, 0x9a, 0x28, 0xc9 } }

class nsIDrawingSurfaceWin : public nsISupports
{
public:
  /**
   * Initialize a drawing surface using a windows DC.
   * aDC is "owned" by the drawing surface until the drawing
   * surface is destroyed.
   * @param  aDC HDC to initialize drawing surface with
   * @return error status
   **/
  NS_IMETHOD Init(HDC aDC) = 0;

  /**
   * Initialize an offscreen drawing surface using a
   * windows DC. aDC is not "owned" by this drawing surface, instead
   * it is used to create a drawing surface compatible
   * with aDC. if width or height are less than zero, aDC will
   * be created with no offscreen bitmap installed.
   * @param  aDC HDC to initialize drawing surface with
   * @param  aWidth width of drawing surface
   * @param  aHeight height of drawing surface
   * @param  aFlags flags used to control type of drawing
   *         surface created
   * @return error status
   **/
  NS_IMETHOD Init(HDC aDC, PRUint32 aWidth, PRUint32 aHeight,
                  PRUint32 aFlags) = 0;

  /**
   * Get a windows DC that represents the drawing surface.
   * GetDC() must be paired with ReleaseDC(). Getting a DC
   * and Lock()ing are mutually exclusive operations.
   * @param  aDC out parameter for HDC
   * @return error status
   **/
  NS_IMETHOD GetDC(HDC *aDC) = 0;

  /**
   * Release a windows DC obtained by GetDC().
   * ReleaseDC() must be preceded by a call to ReleaseDC().
   * @return error status
   **/
  NS_IMETHOD ReleaseDC(void) = 0;

  /**
   * If ReleaseDC() truly destroys the state in the DC
   * this will return PR_TRUE.
   * @param  aDestructive out parameter for destructiveness
   * @return error status
   **/
  NS_IMETHOD IsReleaseDCDestructive(PRBool *aDestructive) = 0;
};

#endif  // nsIDrawingSurfaceWin_h___ 
