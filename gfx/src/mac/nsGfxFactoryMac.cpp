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
 *   Patrick C. Beard <beard@netscape.com>
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

#include "nscore.h"
#include "nsIFactory.h"
#include "nsISupports.h"
#include "nsGfxCIID.h"
#include "nsFontMetricsMac.h"
#include "nsRenderingContextMac.h"
#include "nsImageMac.h"
#include "nsDeviceContextMac.h"
#include "nsRegionMac.h"
#include "nsScriptableRegion.h"
#include "nsBlender.h"
#include "nsCOMPtr.h"
#include "nsUnicodeMappingUtil.h"
#include "gfxImageFrame.h"

#include "nsIGenericFactory.h"


NS_GENERIC_FACTORY_CONSTRUCTOR(nsFontMetricsMac)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsDeviceContextMac)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsRenderingContextMac)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsImageMac)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsRegionMac)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsBlender)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsDrawingSurfaceMac)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsFontEnumeratorMac)
NS_GENERIC_FACTORY_CONSTRUCTOR(gfxImageFrame)

static NS_IMETHODIMP
nsScriptableRegionConstructor(nsISupports* aOuter, REFNSIID aIID, void** aResult)
{
  NS_ENSURE_NO_AGGREGATION(aOuter);
  NS_ENSURE_ARG_POINTER(aResult);
  *aResult = nsnull;
  nsCOMPtr<nsIRegion> region = new nsRegionMac();
  NS_ENSURE_TRUE(region, NS_ERROR_OUT_OF_MEMORY);
  nsCOMPtr<nsIScriptableRegion> result(new nsScriptableRegion(region));
  NS_ENSURE_TRUE(result, NS_ERROR_OUT_OF_MEMORY);
  return result->QueryInterface(aIID, aResult);
}

static const nsModuleComponentInfo components[] =
{
  { "nsFontMetrics",
    NS_FONT_METRICS_CID,
    "@mozilla.org/gfx/fontmetrics;1",
    nsFontMetricsMacConstructor },
  { "nsDeviceContext",
    NS_DEVICE_CONTEXT_CID,
    "@mozilla.org/gfx/devicecontext;1",
    nsDeviceContextMacConstructor },
  { "nsRenderingContext",
    NS_RENDERING_CONTEXT_CID,
    "@mozilla.org/gfx/renderingcontext;1",
    nsRenderingContextMacConstructor },
  { "nsImage",
    NS_IMAGE_CID,
    "@mozilla.org/gfx/image;1",
    nsImageMacConstructor },
  { "nsRegion",
    NS_REGION_CID,
    "@mozilla.org/gfx/unscriptable-region;1",
    nsRegionMacConstructor },
  { "nsScriptableRegion",
    NS_SCRIPTABLE_REGION_CID,
    "@mozilla.org/gfx/region;1",
    nsScriptableRegionConstructor },
  { "nsBlender",
    NS_BLENDER_CID,
    "@mozilla.org/gfx/blender;1",
    nsBlenderConstructor },
  { "nsDrawingSurface",
    NS_DRAWING_SURFACE_CID,
    "@mozilla.org/gfx/drawing-surface;1",
    nsDrawingSurfaceMacConstructor },
  { "nsFontEnumerator",
    NS_FONT_ENUMERATOR_CID,
    "@mozilla.org/gfx/fontenumerator;1",
    nsFontEnumeratorMacConstructor },
  { "windows image frame",
    GFX_IMAGEFRAME_CID,
    "@mozilla.org/gfx/image/frame;2",
    gfxImageFrameConstructor, },
};

PR_STATIC_CALLBACK(void)
nsGfxMacModuleDtor(nsIModule *self)
{
  nsUnicodeMappingUtil::FreeSingleton();
}

NS_IMPL_NSGETMODULE_WITH_DTOR(nsGfxMacModule, components, nsGfxMacModuleDtor)


