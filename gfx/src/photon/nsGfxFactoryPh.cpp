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
 * Christopher Blizzard.
 * Portions created by the Initial Developer are Copyright (C) 2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Christopher Blizzzard <blizzard@mozilla.org>
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

#include "nsIGenericFactory.h"
#include "nsIModule.h"
#include "nsCOMPtr.h"
#include "nsGfxCIID.h"

#include "nsBlender.h"
#include "nsFontMetricsPh.h"
#include "nsRenderingContextPh.h"
#include "nsDeviceContextSpecPh.h"
#include "nsDeviceContextSpecFactoryP.h"
#include "nsScreenManagerPh.h"
#include "nsScriptableRegion.h"
#include "nsDeviceContextPh.h"
#include "nsPrintOptionsPh.h"
#include "nsPrintSession.h"
#include "nsFontList.h"
#include "gfxImageFrame.h"

// objects that just require generic constructors

NS_GENERIC_FACTORY_CONSTRUCTOR(nsFontMetricsPh)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsDeviceContextPh)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsRenderingContextPh)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsImagePh)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsBlender)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsRegionPh)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsDeviceContextSpecPh)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsDeviceContextSpecFactoryPh)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsFontEnumeratorPh)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsFontList)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsScreenManagerPh)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsPrinterEnumeratorPh)
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsPrintOptionsPh, Init)
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsPrintSession, Init)
NS_GENERIC_FACTORY_CONSTRUCTOR(gfxImageFrame)

// our custom constructors

static nsresult nsScriptableRegionConstructor(nsISupports *aOuter, REFNSIID aIID, void **aResult)
{
  nsresult rv;

  nsIScriptableRegion *inst;

  if ( NULL == aResult )
  {
    rv = NS_ERROR_NULL_POINTER;
    return rv;
  }
  *aResult = NULL;
  if (NULL != aOuter)
  {
    rv = NS_ERROR_NO_AGGREGATION;
    return rv;
  }
  // create an nsRegionPh and get the scriptable region from it
  nsCOMPtr <nsIRegion> rgn;
  NS_NEWXPCOM(rgn, nsRegionPh);
  nsCOMPtr<nsIScriptableRegion> scriptableRgn;
  if (rgn != nsnull)
  {
    scriptableRgn = new nsScriptableRegion(rgn);
    inst = scriptableRgn;
  }
  if (NULL == inst)
  {
    rv = NS_ERROR_OUT_OF_MEMORY;
    return rv;
  }
  NS_ADDREF(inst);
  // release our variable above now that we have created our owning
  // reference - we don't want this to go out of scope early!
  scriptableRgn = nsnull;
  rv = inst->QueryInterface(aIID, aResult);
  NS_RELEASE(inst);

  return rv;
}

static const nsModuleComponentInfo components[] =
{
  { "Ph Font Metrics",
    NS_FONT_METRICS_CID,
    "@mozilla.org/gfx/fontmetrics;1",
    nsFontMetricsPhConstructor },

  { "Ph Device Context",
    NS_DEVICE_CONTEXT_CID,
    "@mozilla.org/gfx/devicecontext;1",
    nsDeviceContextPhConstructor },

  { "Ph Rendering Context",
    NS_RENDERING_CONTEXT_CID,
    "@mozilla.org/gfx/renderingcontext;1",
    nsRenderingContextPhConstructor },

  { "Ph Image",
    NS_IMAGE_CID,
    "@mozilla.org/gfx/image;1",
    nsImagePhConstructor },

  { "Ph Region",
    NS_REGION_CID,
    "@mozilla.org/gfx/region/Ph;1",
    nsRegionPhConstructor },

  { "Scriptable Region",
    NS_SCRIPTABLE_REGION_CID,
    "@mozilla.org/gfx/region;1",
    nsScriptableRegionConstructor },

  { "Blender",
    NS_BLENDER_CID,
    "@mozilla.org/gfx/blender;1",
    nsBlenderConstructor },

  { "Ph Device Context Spec",
    NS_DEVICE_CONTEXT_SPEC_CID,
    "@mozilla.org/gfx/devicecontextspec;1",
    nsDeviceContextSpecPhConstructor },

  { "Ph Device Context Spec Factory",
    NS_DEVICE_CONTEXT_SPEC_FACTORY_CID,
    "@mozilla.org/gfx/devicecontextspecfactory;1",
    nsDeviceContextSpecFactoryPhConstructor },

  { "PrintSettings Service",
		NS_PRINTSETTINGSSERVICE_CID,
		"@mozilla.org/gfx/printsettings-service;1",
    nsPrintOptionsPhConstructor },

   { "Ph Font Enumerator",
    NS_FONT_ENUMERATOR_CID,
    "@mozilla.org/gfx/fontenumerator;1",
    nsFontEnumeratorPhConstructor },

	{ "Font List",
		NS_FONTLIST_CID,
		NS_FONTLIST_CONTRACTID,
		nsFontListConstructor },

  { "Ph Screen Manager",
    NS_SCREENMANAGER_CID,
    "@mozilla.org/gfx/screenmanager;1",
    nsScreenManagerPhConstructor },

	{ "Ph Printer Enumerator",
	  NS_PRINTER_ENUMERATOR_CID,
	"@mozilla.org/gfx/printerenumerator;1",
	nsPrinterEnumeratorPhConstructor },

  { "Print Session",
    NS_PRINTSESSION_CID,
    "@mozilla.org/gfx/printsession;1",
    nsPrintSessionConstructor },

  { "windows image frame",
    GFX_IMAGEFRAME_CID,
    "@mozilla.org/gfx/image/frame;2",
    gfxImageFrameConstructor, },
};

NS_IMPL_NSGETMODULE(nsGfxPhModule, components)

