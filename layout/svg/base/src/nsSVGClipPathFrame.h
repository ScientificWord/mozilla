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
 * The Original Code is the Mozilla SVG project.
 *
 * The Initial Developer of the Original Code is IBM Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

#ifndef __NS_SVGCLIPPATHFRAME_H__
#define __NS_SVGCLIPPATHFRAME_H__

#include "nsISupports.h"

class nsISVGRendererCanvas;
class nsISVGRendererSurface;
class nsISVGChildFrame;
class nsIDOMSVGMatrix;
class nsIURI;
class nsIContent;

#define NS_ISVGCLIPPATHFRAME_IID \
{0x9309e388, 0xde9b, 0x4848, {0x84, 0xfe, 0x22, 0xc1, 0xd8, 0x0c, 0x89, 0x11}}

class nsISVGClipPathFrame : public nsISupports {
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_ISVGCLIPPATHFRAME_IID)

  NS_IMETHOD ClipPaint(nsISVGRendererCanvas* canvas,
                       nsISVGRendererSurface* aClipSurface,
                       nsISVGChildFrame* aParent,
                       nsCOMPtr<nsIDOMSVGMatrix> aMatrix) = 0;

  NS_IMETHOD ClipHitTest(nsISVGChildFrame* aParent,
                         nsCOMPtr<nsIDOMSVGMatrix> aMatrix,
                         float aX, float aY, PRBool *aHit) = 0;

  // Check if this clipPath is made up of more than one geometry object.
  // If so, the clipping API in cairo isn't enough and we need to use
  // mask based clipping.

  NS_IMETHOD IsTrivial(PRBool *aTrivial) = 0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsISVGClipPathFrame, NS_ISVGCLIPPATHFRAME_IID)


nsresult
NS_GetSVGClipPathFrame(nsISVGClipPathFrame **aResult,
                       nsIURI *aURI, nsIContent *aContent);

#endif
