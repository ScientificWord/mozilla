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
 * The Initial Developer of the Original Code is
 * Crocodile Clips Ltd..
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Alex Fritze <alex.fritze@crocodile-clips.com> (original author)
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

#ifndef __NS_ISVGGLYPHFRAGMENTLEAF_H__
#define __NS_ISVGGLYPHFRAGMENTLEAF_H__

#include "nsISVGGlyphFragmentNode.h"
#include "nsIDOMSVGLengthList.h"

class nsIDOMSVGPoint;
class nsIDOMSVGRect;
class nsSVGTextPathFrame;

// {2C466AED-CF7B-4479-A807-98151215A645}
#define NS_ISVGGLYPHFRAGMENTLEAF_IID \
{ 0x2c466aed, 0xcf7b, 0x4479, { 0xa8, 0x7, 0x98, 0x15, 0x12, 0x15, 0xa6, 0x45 } }

class nsISVGGlyphFragmentLeaf : public nsISVGGlyphFragmentNode
{
public:

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_ISVGGLYPHFRAGMENTLEAF_IID)

  NS_IMETHOD GetStartPositionOfChar(PRUint32 charnum, nsIDOMSVGPoint **_retval)=0;
  NS_IMETHOD GetEndPositionOfChar(PRUint32 charnum, nsIDOMSVGPoint **_retval)=0;
  NS_IMETHOD GetExtentOfChar(PRUint32 charnum, nsIDOMSVGRect **_retval)=0;
  NS_IMETHOD GetRotationOfChar(PRUint32 charnum, float *_retval)=0;

  enum { BASELINE_ALPHABETIC = 0U };
  enum { BASELINE_HANGING = 1U };
  enum { BASELINE_IDEOGRAPHC = 2U };
  enum { BASELINE_MATHEMATICAL = 3U };
  enum { BASELINE_CENTRAL = 4U };
  enum { BASELINE_MIDDLE = 5U };
  enum { BASELINE_TEXT_BEFORE_EDGE = 6U };
  enum { BASELINE_TEXT_AFTER_EDGE = 7U };

  NS_IMETHOD_(float) GetBaselineOffset(PRUint16 baselineIdentifier)=0;
  NS_IMETHOD_(float) GetAdvance()=0;

  NS_IMETHOD_(void) SetGlyphPosition(float x, float y)=0;
  NS_IMETHOD_(nsSVGTextPathFrame*) FindTextPathParent()=0;
  NS_IMETHOD_(PRBool) IsStartOfChunk()=0; // == is new absolutely positioned chunk.
  NS_IMETHOD_(void) GetAdjustedPosition(/* inout */ float &x, /* inout */ float &y)=0;

  NS_IMETHOD_(already_AddRefed<nsIDOMSVGLengthList>) GetX()=0;
  NS_IMETHOD_(already_AddRefed<nsIDOMSVGLengthList>) GetY()=0;
  NS_IMETHOD_(already_AddRefed<nsIDOMSVGLengthList>) GetDx()=0;
  NS_IMETHOD_(already_AddRefed<nsIDOMSVGLengthList>) GetDy()=0;
  NS_IMETHOD_(PRUint16) GetTextAnchor()=0;
  NS_IMETHOD_(PRBool) IsAbsolutelyPositioned()=0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsISVGGlyphFragmentLeaf,
                              NS_ISVGGLYPHFRAGMENTLEAF_IID)

#endif // __NS_ISVGGLYPHFRAGMENTLEAF_H__
