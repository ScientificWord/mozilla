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
 * The Initial Developer of the Original Code is Crocodile Clips Ltd..
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Alex Fritze <alex.fritze@crocodile-clips.com> (original author)
 *   Jonathan Watt <jonathan.watt@strath.ac.uk>
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

#include "nsSVGLength2.h"
#include "prdtoa.h"
#include "nsTextFormatter.h"
#include "nsSVGSVGElement.h"

NS_IMPL_ADDREF(nsSVGLength2::DOMBaseVal)
NS_IMPL_RELEASE(nsSVGLength2::DOMBaseVal)

NS_IMPL_ADDREF(nsSVGLength2::DOMAnimVal)
NS_IMPL_RELEASE(nsSVGLength2::DOMAnimVal)

NS_IMPL_ADDREF(nsSVGLength2::DOMAnimatedLength)
NS_IMPL_RELEASE(nsSVGLength2::DOMAnimatedLength)

NS_INTERFACE_MAP_BEGIN(nsSVGLength2::DOMBaseVal)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGLength)
  NS_INTERFACE_MAP_ENTRY(nsISupports)
  NS_INTERFACE_MAP_ENTRY_CONTENT_CLASSINFO(SVGLength)
NS_INTERFACE_MAP_END

NS_INTERFACE_MAP_BEGIN(nsSVGLength2::DOMAnimVal)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGLength)
  NS_INTERFACE_MAP_ENTRY(nsISupports)
  NS_INTERFACE_MAP_ENTRY_CONTENT_CLASSINFO(SVGLength)
NS_INTERFACE_MAP_END

NS_INTERFACE_MAP_BEGIN(nsSVGLength2::DOMAnimatedLength)
  NS_INTERFACE_MAP_ENTRY(nsIDOMSVGAnimatedLength)
  NS_INTERFACE_MAP_ENTRY(nsISupports)
  NS_INTERFACE_MAP_ENTRY_CONTENT_CLASSINFO(SVGAnimatedLength)
NS_INTERFACE_MAP_END

static nsIAtom** const unitMap[] =
{
  nsnull, /* SVG_LENGTHTYPE_UNKNOWN */
  nsnull, /* SVG_LENGTHTYPE_NUMBER */
  &nsGkAtoms::percentage,
  &nsGkAtoms::em,
  &nsGkAtoms::ex,
  &nsGkAtoms::px,
  &nsGkAtoms::cm,
  &nsGkAtoms::mm,
  &nsGkAtoms::in,
  &nsGkAtoms::pt,
  &nsGkAtoms::pc
};

/* Helper functions */

static PRBool
IsValidUnitType(PRUint16 unit)
{
  if (unit > nsIDOMSVGLength::SVG_LENGTHTYPE_UNKNOWN &&
      unit <= nsIDOMSVGLength::SVG_LENGTHTYPE_PC)
    return PR_TRUE;

  return PR_FALSE;
}

static void
GetUnitString(nsAString& unit, PRUint16 unitType)
{
  if (IsValidUnitType(unitType)) {
    if (unitMap[unitType]) {
      (*unitMap[unitType])->ToString(unit);
    }
    return;
  }

  NS_NOTREACHED("Unknown unit type");
  return;
}

static PRUint16
GetUnitTypeForString(const char* unitStr)
{
  if (!unitStr || *unitStr == '\0') 
    return nsIDOMSVGLength::SVG_LENGTHTYPE_NUMBER;
                   
  nsCOMPtr<nsIAtom> unitAtom = do_GetAtom(unitStr);

  for (int i = 0 ; i < NS_ARRAY_LENGTH(unitMap) ; i++) {
    if (unitMap[i] && *unitMap[i] == unitAtom) {
      return i;
    }
  }

  return nsIDOMSVGLength::SVG_LENGTHTYPE_UNKNOWN;
}

static void
GetValueString(nsAString &aValueAsString, float aValue, PRUint16 aUnitType)
{
  PRUnichar buf[24];
  nsTextFormatter::snprintf(buf, sizeof(buf)/sizeof(PRUnichar),
                            NS_LITERAL_STRING("%g").get(),
                            (double)aValue);
  aValueAsString.Assign(buf);

  nsAutoString unitString;
  GetUnitString(unitString, aUnitType);
  aValueAsString.Append(unitString);
}

static nsresult
GetValueFromString(const nsAString &aValueAsString,
                   float *aValue,
                   PRUint16 *aUnitType)
{
  NS_ConvertUTF16toUTF8 value(aValueAsString);
  const char *str = value.get();

  if (NS_IsAsciiWhitespace(*str))
    return NS_ERROR_FAILURE;
  
  char *rest;
  *aValue = float(PR_strtod(str, &rest));
  if (rest != str) {
    *aUnitType = GetUnitTypeForString(rest);
    if (IsValidUnitType(*aUnitType)) {
      return NS_OK;
    }
  }
  
  return NS_ERROR_FAILURE;
}

float
nsSVGLength2::GetMMPerPixel(nsSVGSVGElement *aCtx) const
{
  if (!aCtx)
    return 1;

  float mmPerPx = aCtx->GetMMPerPx(mCtxType);

  if (mmPerPx == 0.0f) {
    NS_ASSERTION(mmPerPx != 0.0f, "invalid mm/pixels");
    mmPerPx = 1e-4f; // some small value
  }

  return mmPerPx;
}

float
nsSVGLength2::GetAxisLength(nsSVGSVGElement *aCtx) const
{
  if (!aCtx)
    return 1;

  float d = aCtx->GetLength(mCtxType);

  if (d == 0.0f) {
    NS_WARNING("zero axis length");
    d = 1e-20f;
  }

  return d;
}

float
nsSVGLength2::GetUnitScaleFactor(nsSVGElement *aSVGElement) const
{
  switch (mSpecifiedUnitType) {
  case nsIDOMSVGLength::SVG_LENGTHTYPE_NUMBER:
  case nsIDOMSVGLength::SVG_LENGTHTYPE_PX:
    return 1;
  case nsIDOMSVGLength::SVG_LENGTHTYPE_EMS:
    return 1 / GetEmLength(aSVGElement);
  case nsIDOMSVGLength::SVG_LENGTHTYPE_EXS:
    return 1 / GetExLength(aSVGElement);
  }

  return GetUnitScaleFactor(aSVGElement->GetCtx());
}

float
nsSVGLength2::GetUnitScaleFactor(nsSVGSVGElement *aCtx) const
{
  switch (mSpecifiedUnitType) {
  case nsIDOMSVGLength::SVG_LENGTHTYPE_NUMBER:
  case nsIDOMSVGLength::SVG_LENGTHTYPE_PX:
    return 1;
  case nsIDOMSVGLength::SVG_LENGTHTYPE_MM:
    return GetMMPerPixel(aCtx);
  case nsIDOMSVGLength::SVG_LENGTHTYPE_CM:
    return GetMMPerPixel(aCtx) / 10.0f;
  case nsIDOMSVGLength::SVG_LENGTHTYPE_IN:
    return GetMMPerPixel(aCtx) / 25.4f;
  case nsIDOMSVGLength::SVG_LENGTHTYPE_PT:
    return GetMMPerPixel(aCtx) * 72.0f / 25.4f;
  case nsIDOMSVGLength::SVG_LENGTHTYPE_PC:
    return GetMMPerPixel(aCtx) * 72.0f / 24.4f / 12.0f;
  case nsIDOMSVGLength::SVG_LENGTHTYPE_PERCENTAGE:
    return 100.0f / GetAxisLength(aCtx);
  case nsIDOMSVGLength::SVG_LENGTHTYPE_EMS:
    return 1 / GetEmLength(aCtx);
  case nsIDOMSVGLength::SVG_LENGTHTYPE_EXS:
    return 1 / GetExLength(aCtx);
  default:
    NS_NOTREACHED("Unknown unit type");
    return 0;
  }
}

void
nsSVGLength2::SetBaseValueInSpecifiedUnits(float aValue,
                                           nsSVGElement *aSVGElement)
{
  mBaseVal = aValue;
  aSVGElement->DidChangeLength(mAttrEnum, PR_TRUE);
}

void
nsSVGLength2::ConvertToSpecifiedUnits(PRUint16 unitType,
                                      nsSVGElement *aSVGElement)
{
  if (!IsValidUnitType(unitType))
    return;

  float valueInUserUnits = mBaseVal / GetUnitScaleFactor(aSVGElement);
  mSpecifiedUnitType = PRUint8(unitType);
  SetBaseValue(valueInUserUnits, aSVGElement);
}

void
nsSVGLength2::NewValueSpecifiedUnits(PRUint16 unitType,
                                     float valueInSpecifiedUnits,
                                     nsSVGElement *aSVGElement)
{
  if (!IsValidUnitType(unitType))
    return;

  mBaseVal = mAnimVal = valueInSpecifiedUnits;
  mSpecifiedUnitType = PRUint8(unitType);
  aSVGElement->DidChangeLength(mAttrEnum, PR_TRUE);
}

nsresult
nsSVGLength2::ToDOMBaseVal(nsIDOMSVGLength **aResult, nsSVGElement *aSVGElement)
{
  *aResult = new DOMBaseVal(this, aSVGElement);
  if (!*aResult)
    return NS_ERROR_OUT_OF_MEMORY;

  NS_ADDREF(*aResult);
  return NS_OK;
}

nsresult
nsSVGLength2::ToDOMAnimVal(nsIDOMSVGLength **aResult, nsSVGElement *aSVGElement)
{
  *aResult = new DOMAnimVal(this, aSVGElement);
  if (!*aResult)
    return NS_ERROR_OUT_OF_MEMORY;

  NS_ADDREF(*aResult);
  return NS_OK;
}

/* Implementation */

nsresult
nsSVGLength2::SetBaseValueString(const nsAString &aValueAsString,
                                 nsSVGElement *aSVGElement,
                                 PRBool aDoSetAttr)
{
  float value;
  PRUint16 unitType;
  
  nsresult rv = GetValueFromString(aValueAsString, &value, &unitType);
  if (NS_FAILED(rv)) {
    return rv;
  }
  
  mBaseVal = mAnimVal = value;
  mSpecifiedUnitType = PRUint8(unitType);
  aSVGElement->DidChangeLength(mAttrEnum, aDoSetAttr);

  return NS_OK;
}

void
nsSVGLength2::GetBaseValueString(nsAString & aValueAsString)
{
  GetValueString(aValueAsString, mBaseVal, mSpecifiedUnitType);
}

void
nsSVGLength2::GetAnimValueString(nsAString & aValueAsString)
{
  GetValueString(aValueAsString, mAnimVal, mSpecifiedUnitType);
}

void
nsSVGLength2::SetBaseValue(float aValue, nsSVGElement *aSVGElement)
{
  mAnimVal = mBaseVal = aValue * GetUnitScaleFactor(aSVGElement);
  aSVGElement->DidChangeLength(mAttrEnum, PR_TRUE);
}

nsresult
nsSVGLength2::ToDOMAnimatedLength(nsIDOMSVGAnimatedLength **aResult,
                                  nsSVGElement *aSVGElement)
{
  *aResult = new DOMAnimatedLength(this, aSVGElement);
  if (!*aResult)
    return NS_ERROR_OUT_OF_MEMORY;

  NS_ADDREF(*aResult);
  return NS_OK;
}
