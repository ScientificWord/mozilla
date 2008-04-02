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

#ifndef nsIViewObserver_h___
#define nsIViewObserver_h___

#include "nsISupports.h"
#include "nsEvent.h"
#include "nsColor.h"
#include "nsRect.h"

class nsIRenderingContext;
class nsGUIEvent;

#define NS_IVIEWOBSERVER_IID   \
{ 0x0f4bc34a, 0xc93b, 0x4699, \
{ 0xb6, 0xc2, 0xb3, 0xca, 0x9e, 0xe4, 0x6c, 0x95 } }

class nsIViewObserver : public nsISupports
{
public:
  
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IVIEWOBSERVER_IID)

  /* called when the observer needs to paint. This paints the entire
   * frame subtree rooted at the view, including frame subtrees from
   * subdocuments.
   * @param aRenderingContext rendering context to paint to; the origin
   * of the view is painted at (0,0) in the rendering context's current
   * transform. For best results this should transform to pixel-aligned
   * coordinates.
   * @param aDirtyRegion the region to be painted, in the coordinates of aRootView
   * @return error status
   */
  NS_IMETHOD Paint(nsIView*             aRootView,
                   nsIRenderingContext* aRenderingContext,
                   const nsRegion&      aDirtyRegion) = 0;

  /**
   * @see nsLayoutUtils::ComputeRepaintRegionForCopy
   */
  NS_IMETHOD ComputeRepaintRegionForCopy(nsIView*      aRootView,
                                         nsIView*      aMovingView,
                                         nsPoint       aDelta,
                                         const nsRect& aCopyRect,
                                         nsRegion*     aRepaintRegion) = 0;

  /* called when the observer needs to handle an event
   * @param aView  - where to start processing the event; the root view,
   * or the view that's currently capturing this sort of event; must be a view
   * for this presshell
   * @param aEvent - event notification
   * @param aEventStatus - out parameter for event handling
   *                       status
   * @param aHandled - whether the correct frame was found to
   *                   handle the event
   * @return error status
   */
  NS_IMETHOD HandleEvent(nsIView*       aView,
                         nsGUIEvent*    aEvent,
                         nsEventStatus* aEventStatus) = 0;

  /* called when the view has been resized and the
   * content within the view needs to be reflowed.
   * @param aWidth - new width of view
   * @param aHeight - new height of view
   * @return error status
   */
  NS_IMETHOD ResizeReflow(nsIView * aView, nscoord aWidth, nscoord aHeight) = 0;

  /**
   * Hack to find out if the view observer is itself visible, in lieu
   * of having the view trees linked.
   */
  NS_IMETHOD_(PRBool) IsVisible() = 0;

  /**
   * Notify the observer that we're about to start painting.  This
   * gives the observer a chance to make some last-minute invalidates
   * and geometry changes if it wants to.
   */
  NS_IMETHOD_(void) WillPaint() = 0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsIViewObserver, NS_IVIEWOBSERVER_IID)

#endif
