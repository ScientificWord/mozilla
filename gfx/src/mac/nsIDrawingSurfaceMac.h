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

#ifndef nsIDrawingSurfaceMac_h___
#define nsIDrawingSurfaceMac_h___

#include "nsIDrawingSurface.h"
#include "nsIWidget.h"
#include "nsIRenderingContext.h"
#include <QDOffscreen.h>

class GraphicsState;

// windows specific drawing surface method set

#define NS_IDRAWING_SURFACE_MAC_IID   \
{ 0xd49598bb, 0x04ff, 0x4aba, \
 { 0xab, 0x90, 0x5b, 0xd0, 0xdd, 0x82, 0xba, 0xef } }

class nsIDrawingSurfaceMac : public nsISupports
{
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IDRAWING_SURFACE_MAC_IID)

  /**
   * Initialize a drawing surface using a Macintosh GrafPtr.
   * aPort is not owned by this drawing surface, just used by it.
   * @param  aPort GrafPtr to initialize drawing surface with
   * @return error status
   **/
  NS_IMETHOD Init(nsIDrawingSurface* aDS) = 0;

  /**
   * Initialize a drawing surface using a Macintosh GrafPtr.
   * aPort is not owned by this drawing surface, just used by it.
   * @param  aPort GrafPtr to initialize drawing surface with
   * @return error status
   **/
  NS_IMETHOD Init(CGrafPtr aPort) = 0;

  /**
   * Initialize a drawing surface using a nsIWidget.
   * aTheWidget is not owned by this drawing surface, just used by it.
   * @param  aTheWidget a nsWidget that contains the GrafPtr and all the data needed
   * @return error status
   **/
	NS_IMETHOD Init(nsIWidget *aTheWidget) = 0;

  /**
   * Create and initialize an offscreen drawing surface 
	 * @param  aDepth depth of the offscreen drawing surface
   * @param  aWidth width of the offscreen drawing surface
   * @param  aHeight height of the offscren drawing surface
   * @param  aFlags flags used to control type of drawing surface created
   * @return error status
   **/
  NS_IMETHOD Init(PRUint32 aDepth, PRUint32 aWidth, PRUint32 aHeight,PRUint32 aFlags) = 0;

  /**
   * Get a Macintosh GrafPtr that represents the drawing surface.
   * @param  aPort out parameter for GrafPtr
   * @return error status
   **/
  NS_IMETHOD GetGrafPtr(CGrafPtr *aPort) = 0;

  /**
   * Quartz helper function.  Constructs a Quartz context from the drawing
   * surface's QuickDraw port.  Must be balanced with a call to
   * EndQuartzDrawing().
   * @return Quartz drawing context
   **/
  NS_IMETHOD_(CGContextRef) StartQuartzDrawing() = 0;

  /**
   * Quartz helper function.  Releases Quartz context and resets state of
   * drawing surface for QuickDraw calls.  Must be called when you are done
   * drawing to the Quartz context.
   * @param Quartz drawing context returned by StartQuartzDrawing()
   **/
  NS_IMETHOD_(void) EndQuartzDrawing(CGContextRef aContext) = 0;

};

NS_DEFINE_STATIC_IID_ACCESSOR(nsIDrawingSurfaceMac,
                              NS_IDRAWING_SURFACE_MAC_IID)

#endif  // nsIDrawingSurfaceMac_h___ 
