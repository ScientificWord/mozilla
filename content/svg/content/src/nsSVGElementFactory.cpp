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
 * Portions created by the Initial Developer are Copyright (C) 2001
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

#include "nsCOMPtr.h"
#include "nsContentCreatorFunctions.h"
#include "nsIAtom.h"
#include "nsINodeInfo.h"
#include "nsSVGAtoms.h"
#include "nsContentDLF.h"
#include "nsContentUtils.h"
#include "nsSVGUtils.h"
#include "nsDebug.h"

nsresult
NS_NewSVGPolylineElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGPolygonElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGCircleElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGEllipseElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGLineElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGRectElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGGElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGSVGElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
#ifdef MOZ_SVG_FOREIGNOBJECT
nsresult
NS_NewSVGForeignObjectElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
#endif
nsresult
NS_NewSVGPathElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGTextElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGTSpanElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGImageElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGStyleElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGLinearGradientElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGMetadataElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGRadialGradientElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGStopElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGDefsElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGDescElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGScriptElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGUseElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGSymbolElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGMarkerElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGTitleElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGClipPathElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGTextPathElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGFilterElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGFEGaussianBlurElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGFEComponentTransferElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGFEFuncRElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGFEFuncGElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGFEFuncBElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGFEFuncAElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGFEMergeElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGFEMergeNodeElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGFEOffsetElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGFEUnimplementedMOZElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGPatternElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);
nsresult
NS_NewSVGMaskElement(nsIContent **aResult, nsINodeInfo *aNodeInfo);

nsresult
NS_NewSVGElement(nsIContent** aResult, nsINodeInfo *aNodeInfo)
{
  NS_PRECONDITION(nsSVGUtils::SVGEnabled(),
                  "creating an SVG element while SVG disabled");

  static const char kSVGStyleSheetURI[] = "resource://gre/res/svg.css";

  // this bit of code is to load svg.css on demand
  nsIDocument *doc = aNodeInfo->GetDocument();
  if (doc)
    doc->EnsureCatalogStyleSheet(kSVGStyleSheetURI);

  nsIAtom *name = aNodeInfo->NameAtom();
  
  if (name == nsSVGAtoms::polyline)
    return NS_NewSVGPolylineElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::polygon)
    return NS_NewSVGPolygonElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::circle)
    return NS_NewSVGCircleElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::ellipse)
    return NS_NewSVGEllipseElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::line)
    return NS_NewSVGLineElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::rect)
    return NS_NewSVGRectElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::svg)
    return NS_NewSVGSVGElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::g)
    return NS_NewSVGGElement(aResult, aNodeInfo);
#ifdef MOZ_SVG_FOREIGNOBJECT
  if (name == nsSVGAtoms::foreignObject)
    return NS_NewSVGForeignObjectElement(aResult, aNodeInfo);
#endif
  if (name == nsSVGAtoms::path)
    return NS_NewSVGPathElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::text)
    return NS_NewSVGTextElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::tspan)
    return NS_NewSVGTSpanElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::image)
    return NS_NewSVGImageElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::style)
    return NS_NewSVGStyleElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::linearGradient)
    return NS_NewSVGLinearGradientElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::metadata)
    return NS_NewSVGMetadataElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::radialGradient)
    return NS_NewSVGRadialGradientElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::stop)
    return NS_NewSVGStopElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::defs)
    return NS_NewSVGDefsElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::desc)
    return NS_NewSVGDescElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::script)
    return NS_NewSVGScriptElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::use)
    return NS_NewSVGUseElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::symbol)
    return NS_NewSVGSymbolElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::marker)
    return NS_NewSVGMarkerElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::title)
    return NS_NewSVGTitleElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::clipPath)
    return NS_NewSVGClipPathElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::textPath)
    return NS_NewSVGTextPathElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::filter)
    return NS_NewSVGFilterElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::feGaussianBlur)
    return NS_NewSVGFEGaussianBlurElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::feComponentTransfer)
    return NS_NewSVGFEComponentTransferElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::feFuncR)
    return NS_NewSVGFEFuncRElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::feFuncG)
    return NS_NewSVGFEFuncGElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::feFuncB)
    return NS_NewSVGFEFuncBElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::feFuncA)
    return NS_NewSVGFEFuncAElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::feMerge)
    return NS_NewSVGFEMergeElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::feMergeNode)
    return NS_NewSVGFEMergeNodeElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::feOffset)
    return NS_NewSVGFEOffsetElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::feBlend            ||
      name == nsSVGAtoms::feColorMatrix      ||
      name == nsSVGAtoms::feComposite        ||
      name == nsSVGAtoms::feConvolveMatrix   ||
      name == nsSVGAtoms::feDiffuseLighting  ||
      name == nsSVGAtoms::feDisplacementMap  ||
      name == nsSVGAtoms::feFlood            ||
      name == nsSVGAtoms::feImage            ||
      name == nsSVGAtoms::feMorphology       ||
      name == nsSVGAtoms::feSpecularLighting ||
      name == nsSVGAtoms::feTile             ||
      name == nsSVGAtoms::feTurbulence)
    return NS_NewSVGFEUnimplementedMOZElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::pattern)
    return NS_NewSVGPatternElement(aResult, aNodeInfo);
  if (name == nsSVGAtoms::mask)
    return NS_NewSVGMaskElement(aResult, aNodeInfo);

  // if we don't know what to create, just create a standard xml element:
  return NS_NewXMLElement(aResult, aNodeInfo);
}

