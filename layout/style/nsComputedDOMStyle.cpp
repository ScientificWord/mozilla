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
 *   Daniel Glazman <glazman@netscape.com>
 *   Boris Zbarsky <bzbarsky@mit.edu>
 *   Christopher A. Aillon <christopher@aillon.com>
 *   Mats Palmgren <mats.palmgren@bredband.net>
 *   Christian Biesinger <cbiesinger@web.de>
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

/* DOM object returned from element.getComputedStyle() */

#include "nsComputedDOMStyle.h"

#include "nsDOMError.h"
#include "nsDOMString.h"
#include "nsIDOMCSS2Properties.h"
#include "nsIDOMElement.h"
#include "nsStyleContext.h"
#include "nsIScrollableFrame.h"
#include "nsContentUtils.h"
#include "prprf.h"

#include "nsCSSProps.h"
#include "nsCSSKeywords.h"
#include "nsDOMCSSRect.h"
#include "nsGkAtoms.h"
#include "nsHTMLReflowState.h"
#include "nsThemeConstants.h"
#include "nsStyleUtil.h"

#include "nsPresContext.h"
#include "nsIDocument.h"

#include "nsCSSPseudoElements.h"
#include "nsStyleSet.h"
#include "imgIRequest.h"
#include "nsInspectorCSSUtils.h"
#include "nsLayoutUtils.h"
#include "nsFrameManager.h"

#if defined(DEBUG_bzbarsky) || defined(DEBUG_caillon)
#define DEBUG_ComputedDOMStyle
#endif

/*
 * This is the implementation of the readonly CSSStyleDeclaration that is
 * returned by the getComputedStyle() function.
 */

static nsComputedDOMStyle *sCachedComputedDOMStyle;

nsresult
NS_NewComputedDOMStyle(nsIComputedDOMStyle** aComputedStyle)
{
  NS_ENSURE_ARG_POINTER(aComputedStyle);

  if (sCachedComputedDOMStyle) {
    // There's an unused nsComputedDOMStyle cached, use it.
    // But before we use it, re-initialize the object.

    // Oh yeah baby, placement new!
    *aComputedStyle = new (sCachedComputedDOMStyle) nsComputedDOMStyle();

    sCachedComputedDOMStyle = nsnull;
  } else {
    // No nsComputedDOMStyle cached, create a new one.

    *aComputedStyle = new nsComputedDOMStyle();
    NS_ENSURE_TRUE(*aComputedStyle, NS_ERROR_OUT_OF_MEMORY);
  }

  NS_ADDREF(*aComputedStyle);

  return NS_OK;
}

static nsIFrame*
GetContainingBlockFor(nsIFrame* aFrame) {
  if (!aFrame) {
    return nsnull;
  }
  return nsHTMLReflowState::GetContainingBlockFor(aFrame);
}

nsComputedDOMStyle::nsComputedDOMStyle()
  : mInner(this), mDocumentWeak(nsnull), mOuterFrame(nsnull),
    mInnerFrame(nsnull), mPresShell(nsnull), mAppUnitsPerInch(0)
{
}


nsComputedDOMStyle::~nsComputedDOMStyle()
{
}

void
nsComputedDOMStyle::Shutdown()
{
  // We want to de-allocate without calling the dtor since we
  // already did that manually in doDestroyComputedDOMStyle(),
  // so cast our cached object to something that doesn't know
  // about our dtor.
  delete reinterpret_cast<char*>(sCachedComputedDOMStyle);
  sCachedComputedDOMStyle = nsnull;
}


// QueryInterface implementation for nsComputedDOMStyle
NS_INTERFACE_MAP_BEGIN(nsComputedDOMStyle)
  NS_INTERFACE_MAP_ENTRY(nsIComputedDOMStyle)
  NS_INTERFACE_MAP_ENTRY(nsICSSDeclaration)
  NS_INTERFACE_MAP_ENTRY(nsIDOMCSSStyleDeclaration)
  NS_INTERFACE_MAP_ENTRY_AGGREGATED(nsIDOMCSS2Properties, &mInner)
  NS_INTERFACE_MAP_ENTRY_AGGREGATED(nsIDOMNSCSS2Properties, &mInner)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIComputedDOMStyle)
  NS_INTERFACE_MAP_ENTRY_CONTENT_CLASSINFO(ComputedCSSStyleDeclaration)
NS_INTERFACE_MAP_END


static void doDestroyComputedDOMStyle(nsComputedDOMStyle *aComputedStyle)
{
  if (!sCachedComputedDOMStyle) {
    // The cache is empty, store aComputedStyle in the cache.

    sCachedComputedDOMStyle = aComputedStyle;
    sCachedComputedDOMStyle->~nsComputedDOMStyle();
  } else {
    // The cache is full, delete aComputedStyle

    delete aComputedStyle;
  }
}

NS_IMPL_ADDREF(nsComputedDOMStyle)
NS_IMPL_RELEASE_WITH_DESTROY(nsComputedDOMStyle,
                             doDestroyComputedDOMStyle(this))


NS_IMETHODIMP
nsComputedDOMStyle::Init(nsIDOMElement *aElement,
                         const nsAString& aPseudoElt,
                         nsIPresShell *aPresShell)
{
  NS_ENSURE_ARG_POINTER(aElement);
  NS_ENSURE_ARG_POINTER(aPresShell);

  mDocumentWeak = do_GetWeakReference(aPresShell->GetDocument());

  mContent = do_QueryInterface(aElement);
  if (!mContent) {
    // This should not happen, all our elements support nsIContent!

    return NS_ERROR_FAILURE;
  }

  if (!DOMStringIsNull(aPseudoElt) && !aPseudoElt.IsEmpty() &&
      aPseudoElt.First() == PRUnichar(':')) {
    // deal with two-colon forms of aPseudoElt
    nsAString::const_iterator start, end;
    aPseudoElt.BeginReading(start);
    aPseudoElt.EndReading(end);
    NS_ASSERTION(start != end, "aPseudoElt is not empty!");
    ++start;
    PRBool haveTwoColons = PR_TRUE;
    if (start == end || *start != PRUnichar(':')) {
      --start;
      haveTwoColons = PR_FALSE;
    }
    mPseudo = do_GetAtom(Substring(start, end));
    NS_ENSURE_TRUE(mPseudo, NS_ERROR_OUT_OF_MEMORY);

    // There aren't any non-CSS2 pseudo-elements with a single ':'
    if (!haveTwoColons &&
        !nsCSSPseudoElements::IsCSS2PseudoElement(mPseudo)) {
      // XXXbz I'd really rather we threw an exception or something, but
      // the DOM spec sucks.
      mPseudo = nsnull;
    }
  }

  nsPresContext *presCtx = aPresShell->GetPresContext();
  NS_ENSURE_TRUE(presCtx, NS_ERROR_FAILURE);

  mAppUnitsPerInch = presCtx->AppUnitsPerInch();

  return NS_OK;
}

NS_IMETHODIMP
nsComputedDOMStyle::GetPropertyValue(const nsCSSProperty aPropID,
                                     nsAString& aValue)
{
  // This is mostly to avoid code duplication with GetPropertyCSSValue(); if
  // perf ever becomes an issue here (doubtful), we can look into changing
  // this.
  return GetPropertyValue(
    NS_ConvertASCIItoUTF16(nsCSSProps::GetStringValue(aPropID)),
    aValue);
}

NS_IMETHODIMP
nsComputedDOMStyle::SetPropertyValue(const nsCSSProperty aPropID,
                                     const nsAString& aValue)
{
  return NS_ERROR_DOM_NO_MODIFICATION_ALLOWED_ERR;
}


NS_IMETHODIMP
nsComputedDOMStyle::GetCssText(nsAString& aCssText)
{
  aCssText.Truncate();

  return NS_OK;
}


NS_IMETHODIMP
nsComputedDOMStyle::SetCssText(const nsAString& aCssText)
{
  return NS_ERROR_DOM_NO_MODIFICATION_ALLOWED_ERR;
}


NS_IMETHODIMP
nsComputedDOMStyle::GetLength(PRUint32* aLength)
{
  NS_PRECONDITION(aLength, "Null aLength!  Prepare to die!");

  (void)GetQueryablePropertyMap(aLength);

  return NS_OK;
}


NS_IMETHODIMP
nsComputedDOMStyle::GetParentRule(nsIDOMCSSRule** aParentRule)
{
  NS_ENSURE_ARG_POINTER(aParentRule);
  *aParentRule = nsnull;

  return NS_OK;
}


NS_IMETHODIMP
nsComputedDOMStyle::GetPropertyValue(const nsAString& aPropertyName,
                                     nsAString& aReturn)
{
  nsCOMPtr<nsIDOMCSSValue> val;

  aReturn.Truncate();

  nsresult rv = GetPropertyCSSValue(aPropertyName, getter_AddRefs(val));
  NS_ENSURE_SUCCESS(rv, rv);

  if (val) {
    rv = val->GetCssText(aReturn);
  }

  return rv;
}


NS_IMETHODIMP
nsComputedDOMStyle::GetPropertyCSSValue(const nsAString& aPropertyName,
                                        nsIDOMCSSValue** aReturn)
{
  NS_ENSURE_ARG_POINTER(aReturn);
  *aReturn = nsnull;

  nsCOMPtr<nsIDocument> document = do_QueryReferent(mDocumentWeak);
  NS_ENSURE_TRUE(document, NS_ERROR_NOT_AVAILABLE);

  // Flush _before_ getting the presshell, since that could create a new
  // presshell.  Also note that we want to flush the style on the document
  // we're computing style in, not on the document mContent is in -- the two
  // may be different.
  document->FlushPendingNotifications(Flush_Style);

  mPresShell = document->GetPrimaryShell();
  NS_ENSURE_TRUE(mPresShell && mPresShell->GetPresContext(),
                 NS_ERROR_NOT_AVAILABLE);

  mOuterFrame = mPresShell->GetPrimaryFrameFor(mContent);
  mInnerFrame = mOuterFrame;
  if (!mOuterFrame || mPseudo) {
    // Need to resolve a style context
    mStyleContextHolder =
      nsInspectorCSSUtils::GetStyleContextForContent(mContent,
                                                     mPseudo,
                                                     mPresShell);
    NS_ENSURE_TRUE(mStyleContextHolder, NS_ERROR_OUT_OF_MEMORY);
  } else {
    nsIAtom* type = mOuterFrame->GetType();
    if (type == nsGkAtoms::tableOuterFrame) {
      // If the frame is an outer table frame then we should get the style
      // from the inner table frame.
      mInnerFrame = mOuterFrame->GetFirstChild(nsnull);
      NS_ASSERTION(mInnerFrame, "Outer table must have an inner");
      NS_ASSERTION(!mInnerFrame->GetNextSibling(),
                   "Outer table frames should have just one child, the inner "
                   "table");
    }

    mStyleContextHolder = mInnerFrame->GetStyleContext();
    NS_ASSERTION(mStyleContextHolder, "Frame without style context?");
  }

  nsresult rv = NS_OK;

  nsCSSProperty prop = nsCSSProps::LookupProperty(aPropertyName);

  PRUint32 i = 0;
  PRUint32 length = 0;
  const ComputedStyleMapEntry* propMap = GetQueryablePropertyMap(&length);
  for (; i < length; ++i) {
    if (prop == propMap[i].mProperty) {
      // Call our pointer-to-member-function.
      rv = (this->*(propMap[i].mGetter))(aReturn);
      break;
    }
  }

#ifdef DEBUG_ComputedDOMStyle
  if (i == length) {
    NS_WARNING(PromiseFlatCString(NS_ConvertUTF16toUTF8(aPropertyName) + 
                                  NS_LITERAL_CSTRING(" is not queryable!")).get());
  }
#endif

  if (NS_FAILED(rv)) {
    *aReturn = nsnull;
  }

  mOuterFrame = nsnull;
  mInnerFrame = nsnull;
  mPresShell = nsnull;

  // Release the current style context for it should be re-resolved
  // whenever a frame is not available.
  mStyleContextHolder = nsnull;

  return rv;
}


NS_IMETHODIMP
nsComputedDOMStyle::RemoveProperty(const nsAString& aPropertyName,
                                   nsAString& aReturn)
{
  return NS_ERROR_DOM_NO_MODIFICATION_ALLOWED_ERR;
}


NS_IMETHODIMP
nsComputedDOMStyle::GetPropertyPriority(const nsAString& aPropertyName,
                                        nsAString& aReturn)
{
  aReturn.Truncate();

  return NS_OK;
}


NS_IMETHODIMP
nsComputedDOMStyle::SetProperty(const nsAString& aPropertyName,
                                const nsAString& aValue,
                                const nsAString& aPriority)
{
  return NS_ERROR_DOM_NO_MODIFICATION_ALLOWED_ERR;
}


NS_IMETHODIMP
nsComputedDOMStyle::Item(PRUint32 aIndex, nsAString& aReturn)
{
  aReturn.Truncate();

  PRUint32 length = 0;
  const ComputedStyleMapEntry* propMap = GetQueryablePropertyMap(&length);
  if (aIndex < length) {
    CopyASCIItoUTF16(nsCSSProps::GetStringValue(propMap[aIndex].mProperty),
                    aReturn);
  }

  return NS_OK;
}


// Property getters...

nsresult
nsComputedDOMStyle::GetBinding(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleDisplay* display = GetStyleDisplay();

  if (display->mBinding) {
    val->SetURI(display->mBinding->mURI);
  } else {
    val->SetIdent(nsGkAtoms::none);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetClear(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleDisplay *display = GetStyleDisplay();

  if (display->mBreakType != NS_STYLE_CLEAR_NONE) {
    const nsAFlatCString& clear =
      nsCSSProps::ValueToKeyword(display->mBreakType,
                                 nsCSSProps::kClearKTable);
    val->SetIdent(clear);
  } else {
    val->SetIdent(nsGkAtoms::none);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetCssFloat(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleDisplay* display = GetStyleDisplay();

  if (display->mFloats != NS_STYLE_FLOAT_NONE) {
    const nsAFlatCString& cssFloat =
      nsCSSProps::ValueToKeyword(display->mFloats,
                                 nsCSSProps::kFloatKTable);
    val->SetIdent(cssFloat);
  } else {
    val->SetIdent(nsGkAtoms::none);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetBottom(nsIDOMCSSValue** aValue)
{
  return GetOffsetWidthFor(NS_SIDE_BOTTOM, aValue);
}

nsresult
nsComputedDOMStyle::SetToRGBAColor(nsROCSSPrimitiveValue* aValue,
                                   nscolor aColor)
{
  if (NS_GET_A(aColor) == 0) {
    aValue->SetIdent(nsGkAtoms::transparent);
    return NS_OK;
  }

  nsROCSSPrimitiveValue *red   = GetROCSSPrimitiveValue();
  nsROCSSPrimitiveValue *green = GetROCSSPrimitiveValue();
  nsROCSSPrimitiveValue *blue  = GetROCSSPrimitiveValue();
  nsROCSSPrimitiveValue *alpha  = GetROCSSPrimitiveValue();

  if (red && green && blue && alpha) {
    PRUint8 a = NS_GET_A(aColor);
    nsDOMCSSRGBColor *rgbColor =
      new nsDOMCSSRGBColor(red, green, blue, alpha, a < 255);

    if (rgbColor) {
      red->SetNumber(NS_GET_R(aColor));
      green->SetNumber(NS_GET_G(aColor));
      blue->SetNumber(NS_GET_B(aColor));
      alpha->SetNumber(nsStyleUtil::ColorComponentToFloat(a));

      aValue->SetColor(rgbColor);
      return NS_OK;
    }
  }

  delete red;
  delete green;
  delete blue;
  delete alpha;

  return NS_ERROR_OUT_OF_MEMORY;
}

nsresult
nsComputedDOMStyle::GetColor(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleColor* color = GetStyleColor();

  nsresult rv = SetToRGBAColor(val, color->mColor);
  if (NS_FAILED(rv)) {
    delete val;
    return rv;
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetOpacity(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  val->SetNumber(GetStyleDisplay()->mOpacity);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetColumnCount(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleColumn* column = GetStyleColumn();

  if (column->mColumnCount == NS_STYLE_COLUMN_COUNT_AUTO) {
    val->SetIdent(nsGkAtoms::_auto);
  } else {
    val->SetNumber(column->mColumnCount);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetColumnWidth(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  // XXX fix the auto case. When we actually have a column frame, I think
  // we should return the computed column width.
  SetValueToCoord(val, GetStyleColumn()->mColumnWidth);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetColumnGap(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleColumn* column = GetStyleColumn();
  if (column->mColumnGap.GetUnit() == eStyleUnit_Normal) {
    val->SetAppUnits(GetStyleFont()->mFont.size);
  } else {
    SetValueToCoord(val, GetStyleColumn()->mColumnGap);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetContent(nsIDOMCSSValue** aValue)
{
  const nsStyleContent *content = GetStyleContent();

  if (content->ContentCount() == 0) {
    nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
    NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);
    val->SetIdent(nsGkAtoms::none);
    return CallQueryInterface(val, aValue);
  }

  if (content->ContentCount() == 1 &&
      content->ContentAt(0).mType == eStyleContentType_AltContent) {
    nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
    NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);
    val->SetIdent(eCSSKeyword__moz_alt_content);
    return CallQueryInterface(val, aValue);
  }

  nsDOMCSSValueList *valueList = GetROCSSValueList(PR_FALSE);
  NS_ENSURE_TRUE(valueList, NS_ERROR_OUT_OF_MEMORY);

  for (PRUint32 i = 0, i_end = content->ContentCount(); i < i_end; ++i) {
    nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
    if (!val || !valueList->AppendCSSValue(val)) {
      delete valueList;
      delete val;
      return NS_ERROR_OUT_OF_MEMORY;
    }

    const nsStyleContentData &data = content->ContentAt(i);
    switch (data.mType) {
      case eStyleContentType_String:
        {
          nsString str;
          nsStyleUtil::EscapeCSSString(nsDependentString(data.mContent.mString),
                                       str);
          str.Insert(PRUnichar('"'), 0);
          str.Append(PRUnichar('"'));
          val->SetString(str);
        }
        break;
      case eStyleContentType_Image:
        {
          nsCOMPtr<nsIURI> uri;
          if (data.mContent.mImage) {
            data.mContent.mImage->GetURI(getter_AddRefs(uri));
          }
          val->SetURI(uri);
        }
        break;
      case eStyleContentType_Attr:
        val->SetString(nsDependentString(data.mContent.mString),
                       nsIDOMCSSPrimitiveValue::CSS_ATTR);
        break;
      case eStyleContentType_Counter:
      case eStyleContentType_Counters:
        {
          /* FIXME: counters should really use an object */
          nsAutoString str;
          if (data.mType == eStyleContentType_Counter) {
            str.AppendLiteral("counter(");
          }
          else {
            str.AppendLiteral("counters(");
          }
          // WRITE ME
          nsCSSValue::Array *a = data.mContent.mCounters;

          str.Append(a->Item(0).GetStringBufferValue());
          PRInt32 typeItem = 1;
          if (data.mType == eStyleContentType_Counters) {
            typeItem = 2;
            str.AppendLiteral(", \"");
            nsString itemstr;
            nsStyleUtil::EscapeCSSString(
              nsDependentString(a->Item(1).GetStringBufferValue()), itemstr);
            str.Append(itemstr);
            str.Append(PRUnichar('"'));
          }
          PRInt32 type = a->Item(typeItem).GetIntValue();
          if (type != NS_STYLE_LIST_STYLE_DECIMAL) {
            str.AppendLiteral(", ");
            str.AppendInt(type);
          }

          str.Append(PRUnichar(')'));
          val->SetString(str, nsIDOMCSSPrimitiveValue::CSS_COUNTER);
        }
        break;
      case eStyleContentType_OpenQuote:
        val->SetIdent(eCSSKeyword_open_quote);
        break;
      case eStyleContentType_CloseQuote:
        val->SetIdent(eCSSKeyword_close_quote);
        break;
      case eStyleContentType_NoOpenQuote:
        val->SetIdent(eCSSKeyword_no_open_quote);
        break;
      case eStyleContentType_NoCloseQuote:
        val->SetIdent(eCSSKeyword_no_close_quote);
        break;
      case eStyleContentType_AltContent:
      default:
        NS_NOTREACHED("unexpected type");
        break;
    }
  }

  return CallQueryInterface(valueList, aValue);
}

nsresult
nsComputedDOMStyle::GetCounterIncrement(nsIDOMCSSValue** aValue)
{
  const nsStyleContent *content = GetStyleContent();

  if (content->CounterIncrementCount() == 0) {
    nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
    NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);
    val->SetIdent(nsGkAtoms::none);
    return CallQueryInterface(val, aValue);
  }

  nsDOMCSSValueList *valueList = GetROCSSValueList(PR_FALSE);
  NS_ENSURE_TRUE(valueList, NS_ERROR_OUT_OF_MEMORY);

  for (PRUint32 i = 0, i_end = content->CounterIncrementCount(); i < i_end; ++i) {
    nsROCSSPrimitiveValue* name = GetROCSSPrimitiveValue();
    if (!name || !valueList->AppendCSSValue(name)) {
      delete valueList;
      delete name;
      return NS_ERROR_OUT_OF_MEMORY;
    }
    
    nsROCSSPrimitiveValue* value = GetROCSSPrimitiveValue();
    if (!value || !valueList->AppendCSSValue(value)) {
      delete valueList;
      delete value;
      return NS_ERROR_OUT_OF_MEMORY;
    }

    const nsStyleCounterData *data = content->GetCounterIncrementAt(i);
    name->SetString(data->mCounter);
    value->SetNumber(data->mValue); // XXX This should really be integer
  }

  return CallQueryInterface(valueList, aValue);
}

nsresult
nsComputedDOMStyle::GetCounterReset(nsIDOMCSSValue** aValue)
{
  const nsStyleContent *content = GetStyleContent();

  if (content->CounterResetCount() == 0) {
    nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
    NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);
    val->SetIdent(nsGkAtoms::none);
    return CallQueryInterface(val, aValue);
  }

  nsDOMCSSValueList *valueList = GetROCSSValueList(PR_FALSE);
  NS_ENSURE_TRUE(valueList, NS_ERROR_OUT_OF_MEMORY);

  for (PRUint32 i = 0, i_end = content->CounterResetCount(); i < i_end; ++i) {
    nsROCSSPrimitiveValue* name = GetROCSSPrimitiveValue();
    if (!name || !valueList->AppendCSSValue(name)) {
      delete valueList;
      delete name;
      return NS_ERROR_OUT_OF_MEMORY;
    }
    
    nsROCSSPrimitiveValue* value = GetROCSSPrimitiveValue();
    if (!value || !valueList->AppendCSSValue(value)) {
      delete valueList;
      delete value;
      return NS_ERROR_OUT_OF_MEMORY;
    }

    const nsStyleCounterData *data = content->GetCounterResetAt(i);
    name->SetString(data->mCounter);
    value->SetNumber(data->mValue); // XXX This should really be integer
  }

  return CallQueryInterface(valueList, aValue);
}

nsresult
nsComputedDOMStyle::GetQuotes(nsIDOMCSSValue** aValue)
{
  const nsStyleQuotes *quotes = GetStyleQuotes();

  if (quotes->QuotesCount() == 0) {
    nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
    NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);
    val->SetIdent(nsGkAtoms::none);
    return CallQueryInterface(val, aValue);
  }

  nsDOMCSSValueList *valueList = GetROCSSValueList(PR_FALSE);
  NS_ENSURE_TRUE(valueList, NS_ERROR_OUT_OF_MEMORY);

  for (PRUint32 i = 0, i_end = quotes->QuotesCount(); i < i_end; ++i) {
    nsROCSSPrimitiveValue* openVal = GetROCSSPrimitiveValue();
    if (!openVal || !valueList->AppendCSSValue(openVal)) {
      delete valueList;
      delete openVal;
      return NS_ERROR_OUT_OF_MEMORY;
    }
    
    nsROCSSPrimitiveValue* closeVal = GetROCSSPrimitiveValue();
    if (!closeVal || !valueList->AppendCSSValue(closeVal)) {
      delete valueList;
      delete closeVal;
      return NS_ERROR_OUT_OF_MEMORY;
    }

    nsString s;
    nsStyleUtil::EscapeCSSString(*quotes->OpenQuoteAt(i), s);
    s.Insert(PRUnichar('"'), 0);
    s.Append(PRUnichar('"'));
    openVal->SetString(s);
    nsStyleUtil::EscapeCSSString(*quotes->CloseQuoteAt(i), s);
    s.Insert(PRUnichar('"'), 0);
    s.Append(PRUnichar('"'));
    closeVal->SetString(s);
  }

  return CallQueryInterface(valueList, aValue);
}

nsresult
nsComputedDOMStyle::GetFontFamily(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleFont* font = GetStyleFont();

  nsCOMPtr<nsIDocument> doc = do_QueryReferent(mDocumentWeak);
  NS_ASSERTION(doc, "document is required");
  nsIPresShell* presShell = doc->GetPrimaryShell();
  NS_ASSERTION(presShell, "pres shell is required");
  nsPresContext *presContext = presShell->GetPresContext();
  NS_ASSERTION(presContext, "pres context is required");

  const nsString& fontName = font->mFont.name;
  PRUint8 generic = font->mFlags & NS_STYLE_FONT_FACE_MASK;
  if (generic == kGenericFont_NONE && !font->mFont.systemFont) { 
    const nsFont* defaultFont =
      presContext->GetDefaultFont(kPresContext_DefaultVariableFont_ID);

    PRInt32 lendiff = fontName.Length() - defaultFont->name.Length();
    if (lendiff > 0) {
      val->SetString(Substring(fontName, 0, lendiff-1)); // -1 removes comma
    } else {
      val->SetString(fontName);
    }
  } else {
    val->SetString(fontName);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetFontSize(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  // Note: GetStyleFont()->mSize is the 'computed size';
  // GetStyleFont()->mFont.size is the 'actual size'
  val->SetAppUnits(GetStyleFont()->mSize);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetFontSizeAdjust(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleFont *font = GetStyleFont();

  if (font->mFont.sizeAdjust) {
    val->SetNumber(font->mFont.sizeAdjust);
  } else {
    val->SetIdent(nsGkAtoms::none);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetFontStyle(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleFont* font = GetStyleFont();

  if (font->mFont.style != NS_STYLE_FONT_STYLE_NORMAL) {
    const nsAFlatCString& style=
      nsCSSProps::ValueToKeyword(font->mFont.style,
                                 nsCSSProps::kFontStyleKTable);
    val->SetIdent(style);
  } else {
    val->SetIdent(nsGkAtoms::normal);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetFontWeight(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleFont* font = GetStyleFont();

  const nsAFlatCString& str_weight=
    nsCSSProps::ValueToKeyword(font->mFont.weight,
                               nsCSSProps::kFontWeightKTable);
  if (!str_weight.IsEmpty()) {
    val->SetIdent(str_weight);
  } else {
    val->SetNumber(font->mFont.weight);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetFontVariant(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleFont* font = GetStyleFont();

  if (font->mFont.variant != NS_STYLE_FONT_VARIANT_NORMAL) {
    const nsAFlatCString& variant=
      nsCSSProps::ValueToKeyword(font->mFont.variant,
                                 nsCSSProps::kFontVariantKTable);
    val->SetIdent(variant);
  } else {
    val->SetIdent(nsGkAtoms::normal);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetBackgroundAttachment(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleBackground *background = GetStyleBackground();

  const nsAFlatCString& backgroundAttachment =
    nsCSSProps::ValueToKeyword(background->mBackgroundAttachment,
                               nsCSSProps::kBackgroundAttachmentKTable);
  val->SetIdent(backgroundAttachment);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetBackgroundClip(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsAFlatCString& backgroundClip =
    nsCSSProps::ValueToKeyword(GetStyleBackground()->mBackgroundClip,
                               nsCSSProps::kBackgroundClipKTable);

  val->SetIdent(backgroundClip);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetBackgroundColor(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleBackground* color = GetStyleBackground();

  if (color->mBackgroundFlags & NS_STYLE_BG_COLOR_TRANSPARENT) {
    const nsAFlatCString& backgroundColor =
      nsCSSProps::ValueToKeyword(NS_STYLE_BG_COLOR_TRANSPARENT,
                                 nsCSSProps::kBackgroundColorKTable);
    val->SetIdent(backgroundColor);
  } else {
    nsresult rv = SetToRGBAColor(val, color->mBackgroundColor);
    if (NS_FAILED(rv)) {
      delete val;
      return rv;
    }
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetBackgroundImage(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleBackground* color = GetStyleBackground();

  if (color->mBackgroundFlags & NS_STYLE_BG_IMAGE_NONE) {
    val->SetIdent(nsGkAtoms::none);
  } else {
    nsCOMPtr<nsIURI> uri;
    if (color->mBackgroundImage) {
      color->mBackgroundImage->GetURI(getter_AddRefs(uri));
    }
    val->SetURI(uri);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetBackgroundInlinePolicy(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsAFlatCString& backgroundPolicy =
      nsCSSProps::ValueToKeyword(GetStyleBackground()->mBackgroundInlinePolicy,
                                 nsCSSProps::kBackgroundInlinePolicyKTable);

  val->SetIdent(backgroundPolicy);

  return CallQueryInterface(val, aValue);  
}

nsresult
nsComputedDOMStyle::GetBackgroundOrigin(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsAFlatCString& backgroundOrigin =
    nsCSSProps::ValueToKeyword(GetStyleBackground()->mBackgroundOrigin,
                               nsCSSProps::kBackgroundOriginKTable);

  val->SetIdent(backgroundOrigin);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetBackgroundPosition(nsIDOMCSSValue** aValue)
{
  nsDOMCSSValueList *valueList = GetROCSSValueList(PR_FALSE);
  NS_ENSURE_TRUE(valueList, NS_ERROR_OUT_OF_MEMORY);

  nsROCSSPrimitiveValue *valX = GetROCSSPrimitiveValue();
  if (!valX || !valueList->AppendCSSValue(valX)) {
    delete valueList;
    delete valX;
    return NS_ERROR_OUT_OF_MEMORY;
  }

  nsROCSSPrimitiveValue *valY = GetROCSSPrimitiveValue();
  if (!valY || !valueList->AppendCSSValue(valY)) {
    delete valueList;
    delete valY;
    return NS_ERROR_OUT_OF_MEMORY;
  }

  const nsStyleBackground *bg = GetStyleBackground();

  if (NS_STYLE_BG_X_POSITION_LENGTH & bg->mBackgroundFlags) {
    valX->SetAppUnits(bg->mBackgroundXPosition.mCoord);
  }
  else if (NS_STYLE_BG_X_POSITION_PERCENT & bg->mBackgroundFlags) {
    valX->SetPercent(bg->mBackgroundXPosition.mFloat);
  }
  else {
    valX->SetPercent(0.0f);
  }

  if (NS_STYLE_BG_Y_POSITION_LENGTH & bg->mBackgroundFlags) {
    valY->SetAppUnits(bg->mBackgroundYPosition.mCoord);
  }
  else if (NS_STYLE_BG_Y_POSITION_PERCENT & bg->mBackgroundFlags) {
    valY->SetPercent(bg->mBackgroundYPosition.mFloat);
  }
  else {
    valY->SetPercent(0.0f);
  }

  return CallQueryInterface(valueList, aValue);  
}

nsresult
nsComputedDOMStyle::GetBackgroundRepeat(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsAFlatCString& backgroundRepeat =
    nsCSSProps::ValueToKeyword(GetStyleBackground()->mBackgroundRepeat,
                               nsCSSProps::kBackgroundRepeatKTable);
  val->SetIdent(backgroundRepeat);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetPadding(nsIDOMCSSValue** aValue)
{
  // return null per spec.
  aValue = nsnull;

  return NS_OK;
}

nsresult
nsComputedDOMStyle::GetPaddingTop(nsIDOMCSSValue** aValue)
{
  return GetPaddingWidthFor(NS_SIDE_TOP, aValue);
}

nsresult
nsComputedDOMStyle::GetPaddingBottom(nsIDOMCSSValue** aValue)
{
  return GetPaddingWidthFor(NS_SIDE_BOTTOM, aValue);
}

nsresult
nsComputedDOMStyle::GetPaddingLeft(nsIDOMCSSValue** aValue)
{
  return GetPaddingWidthFor(NS_SIDE_LEFT, aValue);
}

nsresult
nsComputedDOMStyle::GetPaddingRight(nsIDOMCSSValue** aValue)
{
  return GetPaddingWidthFor(NS_SIDE_RIGHT, aValue);
}

nsresult
nsComputedDOMStyle::GetBorderCollapse(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsAFlatCString& ident=
    nsCSSProps::ValueToKeyword(GetStyleTableBorder()->mBorderCollapse,
                               nsCSSProps::kBorderCollapseKTable);
  val->SetIdent(ident);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetBorderSpacing(nsIDOMCSSValue** aValue)
{
  nsDOMCSSValueList *valueList = GetROCSSValueList(PR_FALSE);
  NS_ENSURE_TRUE(valueList, NS_ERROR_OUT_OF_MEMORY);

  nsROCSSPrimitiveValue* xSpacing = GetROCSSPrimitiveValue();
  if (!xSpacing) {
    delete valueList;
    return NS_ERROR_OUT_OF_MEMORY;
  }
  if (!valueList->AppendCSSValue(xSpacing)) {
    delete valueList;
    delete xSpacing;
    return NS_ERROR_OUT_OF_MEMORY;
  }
    
  nsROCSSPrimitiveValue* ySpacing = GetROCSSPrimitiveValue();
  if (!ySpacing) {
    delete valueList;
    return NS_ERROR_OUT_OF_MEMORY;
  }
  if (!valueList->AppendCSSValue(ySpacing)) {
    delete valueList;
    delete ySpacing;
    return NS_ERROR_OUT_OF_MEMORY;
  }

  const nsStyleTableBorder *border = GetStyleTableBorder();
  // border-spacing will always be a coord
  xSpacing->SetAppUnits(border->mBorderSpacingX.GetCoordValue());
  ySpacing->SetAppUnits(border->mBorderSpacingY.GetCoordValue());

  return CallQueryInterface(valueList, aValue);
}

nsresult
nsComputedDOMStyle::GetCaptionSide(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsAFlatCString& side =
    nsCSSProps::ValueToKeyword(GetStyleTableBorder()->mCaptionSide,
                               nsCSSProps::kCaptionSideKTable);
  val->SetIdent(side);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetEmptyCells(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsAFlatCString& emptyCells =
    nsCSSProps::ValueToKeyword(GetStyleTableBorder()->mEmptyCells,
                               nsCSSProps::kEmptyCellsKTable);
  val->SetIdent(emptyCells);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetTableLayout(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleTable *table = GetStyleTable();

  if (table->mLayoutStrategy != NS_STYLE_TABLE_LAYOUT_AUTO) {
    const nsAFlatCString& tableLayout =
      nsCSSProps::ValueToKeyword(table->mLayoutStrategy,
                                 nsCSSProps::kTableLayoutKTable);
    val->SetIdent(tableLayout);
  } else {
    val->SetIdent(nsGkAtoms::_auto);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetBorderStyle(nsIDOMCSSValue** aValue)
{
  // return null per spec.
  aValue = nsnull;

  return NS_OK;
}

nsresult
nsComputedDOMStyle::GetBorderTopStyle(nsIDOMCSSValue** aValue)
{
  return GetBorderStyleFor(NS_SIDE_TOP, aValue);
}

nsresult
nsComputedDOMStyle::GetBorderBottomStyle(nsIDOMCSSValue** aValue)
{
  return GetBorderStyleFor(NS_SIDE_BOTTOM, aValue);
}
nsresult
nsComputedDOMStyle::GetBorderLeftStyle(nsIDOMCSSValue** aValue)
{
  return GetBorderStyleFor(NS_SIDE_LEFT, aValue);
}

nsresult
nsComputedDOMStyle::GetBorderRightStyle(nsIDOMCSSValue** aValue)
{
  return GetBorderStyleFor(NS_SIDE_RIGHT, aValue);
}

nsresult
nsComputedDOMStyle::GetBorderBottomColors(nsIDOMCSSValue** aValue)
{
  return GetBorderColorsFor(NS_SIDE_BOTTOM, aValue);
}

nsresult
nsComputedDOMStyle::GetBorderLeftColors(nsIDOMCSSValue** aValue)
{
  return GetBorderColorsFor(NS_SIDE_LEFT, aValue);
}

nsresult
nsComputedDOMStyle::GetBorderRightColors(nsIDOMCSSValue** aValue)
{
  return GetBorderColorsFor(NS_SIDE_RIGHT, aValue);
}


nsresult
nsComputedDOMStyle::GetBorderTopColors(nsIDOMCSSValue** aValue)
{
  return GetBorderColorsFor(NS_SIDE_TOP, aValue);
}


nsresult
nsComputedDOMStyle::GetBorderRadiusBottomLeft(nsIDOMCSSValue** aValue)
{
  return GetBorderRadiusFor(NS_SIDE_LEFT, aValue);
}

nsresult
nsComputedDOMStyle::GetBorderRadiusBottomRight(nsIDOMCSSValue** aValue)
{
  return GetBorderRadiusFor(NS_SIDE_BOTTOM, aValue);
}

nsresult
nsComputedDOMStyle::GetBorderRadiusTopLeft(nsIDOMCSSValue** aValue)
{
  return GetBorderRadiusFor(NS_SIDE_TOP, aValue);
}

nsresult
nsComputedDOMStyle::GetBorderRadiusTopRight(nsIDOMCSSValue** aValue)
{
  return GetBorderRadiusFor(NS_SIDE_RIGHT, aValue);
}

nsresult
nsComputedDOMStyle::GetBorderWidth(nsIDOMCSSValue** aValue)
{
  // return null per spec.
  aValue = nsnull;

  return NS_OK;
}

nsresult
nsComputedDOMStyle::GetBorderTopWidth(nsIDOMCSSValue** aValue)
{
  return GetBorderWidthFor(NS_SIDE_TOP, aValue);
}

nsresult
nsComputedDOMStyle::GetBorderBottomWidth(nsIDOMCSSValue** aValue)
{
  return GetBorderWidthFor(NS_SIDE_BOTTOM, aValue);
}

nsresult
nsComputedDOMStyle::GetBorderLeftWidth(nsIDOMCSSValue** aValue)
{
  return GetBorderWidthFor(NS_SIDE_LEFT, aValue);
}

nsresult
nsComputedDOMStyle::GetBorderRightWidth(nsIDOMCSSValue** aValue)
{
  return GetBorderWidthFor(NS_SIDE_RIGHT, aValue);
}

nsresult
nsComputedDOMStyle::GetBorderTopColor(nsIDOMCSSValue** aValue)
{
  return GetBorderColorFor(NS_SIDE_TOP, aValue);
}

nsresult
nsComputedDOMStyle::GetBorderBottomColor(nsIDOMCSSValue** aValue)
{
  return GetBorderColorFor(NS_SIDE_BOTTOM, aValue);
}

nsresult
nsComputedDOMStyle::GetBorderLeftColor(nsIDOMCSSValue** aValue)
{
  return GetBorderColorFor(NS_SIDE_LEFT, aValue);
}

nsresult
nsComputedDOMStyle::GetBorderRightColor(nsIDOMCSSValue** aValue)
{
  return GetBorderColorFor(NS_SIDE_RIGHT, aValue);
}

nsresult
nsComputedDOMStyle::GetMarginWidth(nsIDOMCSSValue** aValue)
{
  // return null per spec.
  aValue = nsnull;

  return NS_OK;
}

nsresult
nsComputedDOMStyle::GetMarginTopWidth(nsIDOMCSSValue** aValue)
{
  return GetMarginWidthFor(NS_SIDE_TOP, aValue);
}

nsresult
nsComputedDOMStyle::GetMarginBottomWidth(nsIDOMCSSValue** aValue)
{
  return GetMarginWidthFor(NS_SIDE_BOTTOM, aValue);
}

nsresult
nsComputedDOMStyle::GetMarginLeftWidth(nsIDOMCSSValue** aValue)
{
  return GetMarginWidthFor(NS_SIDE_LEFT, aValue);
}

nsresult
nsComputedDOMStyle::GetMarginRightWidth(nsIDOMCSSValue** aValue)
{
  return GetMarginWidthFor(NS_SIDE_RIGHT, aValue);
}

nsresult
nsComputedDOMStyle::GetMarkerOffset(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  SetValueToCoord(val, GetStyleContent()->mMarkerOffset);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetOutline(nsIDOMCSSValue** aValue)
{
  // return null per spec.
  aValue = nsnull;

  return NS_OK;
}

nsresult
nsComputedDOMStyle::GetOutlineWidth(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleOutline* outline = GetStyleOutline();

  nsStyleCoord coord;
  PRUint8 outlineStyle = outline->GetOutlineStyle();
  if (outlineStyle == NS_STYLE_BORDER_STYLE_NONE) {
    coord.SetCoordValue(0);
  } else {
    coord = outline->mOutlineWidth;
  }
  SetValueToCoord(val, coord, nsnull, nsCSSProps::kBorderWidthKTable);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetOutlineStyle(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  PRUint8 outlineStyle = GetStyleOutline()->GetOutlineStyle();
  switch (outlineStyle) {
  case NS_STYLE_BORDER_STYLE_NONE:
    val->SetIdent(nsGkAtoms::none);
    break;
  case NS_STYLE_BORDER_STYLE_AUTO:
    val->SetIdent(nsGkAtoms::_auto);
    break;
  default:
    const nsAFlatCString& style =
      nsCSSProps::ValueToKeyword(outlineStyle,
                                 nsCSSProps::kOutlineStyleKTable);
    val->SetIdent(style);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetOutlineOffset(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  SetValueToCoord(val, GetStyleOutline()->mOutlineOffset);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetOutlineRadiusBottomLeft(nsIDOMCSSValue** aValue)
{
  return GetOutlineRadiusFor(NS_SIDE_LEFT, aValue);
}

nsresult
nsComputedDOMStyle::GetOutlineRadiusBottomRight(nsIDOMCSSValue** aValue)
{
  return GetOutlineRadiusFor(NS_SIDE_BOTTOM, aValue);
}

nsresult
nsComputedDOMStyle::GetOutlineRadiusTopLeft(nsIDOMCSSValue** aValue)
{
  return GetOutlineRadiusFor(NS_SIDE_TOP, aValue);
}

nsresult
nsComputedDOMStyle::GetOutlineRadiusTopRight(nsIDOMCSSValue** aValue)
{
  return GetOutlineRadiusFor(NS_SIDE_RIGHT, aValue);
}

nsresult
nsComputedDOMStyle::GetOutlineColor(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  nscolor color;
#ifdef GFX_HAS_INVERT
  GetStyleOutline()->GetOutlineColor(color);
#else
  if (!GetStyleOutline()->GetOutlineColor(color))
    color = GetStyleColor()->mColor;
#endif

  nsresult rv = SetToRGBAColor(val, color);
  if (NS_FAILED(rv)) {
    delete val;
    return rv;
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetOutlineRadiusFor(PRUint8 aSide, nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  nsStyleCoord coord;
  SetValueToCoord(val, GetStyleOutline()->mOutlineRadius.Get(aSide, coord),
                  &nsComputedDOMStyle::GetFrameBorderRectWidth);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetZIndex(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  SetValueToCoord(val, GetStylePosition()->mZIndex);
    
  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetListStyleImage(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleList* list = GetStyleList();

  if (!list->mListStyleImage) {
    val->SetIdent(nsGkAtoms::none);
  } else {
    nsCOMPtr<nsIURI> uri;
    if (list->mListStyleImage) {
      list->mListStyleImage->GetURI(getter_AddRefs(uri));
    }
    val->SetURI(uri);
  }
    
  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetListStylePosition(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsAFlatCString& style =
    nsCSSProps::ValueToKeyword(GetStyleList()->mListStylePosition,
                               nsCSSProps::kListStylePositionKTable);
  val->SetIdent(style);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetListStyleType(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleList *list = GetStyleList();

  if (list->mListStyleType == NS_STYLE_LIST_STYLE_NONE) {
    val->SetIdent(nsGkAtoms::none);
  } else {
    const nsAFlatCString& style =
      nsCSSProps::ValueToKeyword(list->mListStyleType,
                                 nsCSSProps::kListStyleKTable);
    val->SetIdent(style);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetImageRegion(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleList* list = GetStyleList();

  nsresult rv = NS_OK;
  nsROCSSPrimitiveValue *topVal = nsnull;
  nsROCSSPrimitiveValue *rightVal = nsnull;
  nsROCSSPrimitiveValue *bottomVal = nsnull;
  nsROCSSPrimitiveValue *leftVal = nsnull;
  if (list->mImageRegion.width <= 0 || list->mImageRegion.height <= 0) {
    val->SetIdent(nsGkAtoms::_auto);
  } else {
    // create the cssvalues for the sides, stick them in the rect object
    topVal = GetROCSSPrimitiveValue();
    rightVal = GetROCSSPrimitiveValue();
    bottomVal = GetROCSSPrimitiveValue();
    leftVal = GetROCSSPrimitiveValue();
    if (topVal && rightVal && bottomVal && leftVal) {
      nsDOMCSSRect * domRect = new nsDOMCSSRect(topVal, rightVal,
                                                bottomVal, leftVal);
      if (domRect) {
        topVal->SetAppUnits(list->mImageRegion.y);
        rightVal->SetAppUnits(list->mImageRegion.width + list->mImageRegion.x);
        bottomVal->SetAppUnits(list->mImageRegion.height + list->mImageRegion.y);
        leftVal->SetAppUnits(list->mImageRegion.x);
        val->SetRect(domRect);
      } else {
        rv = NS_ERROR_OUT_OF_MEMORY;
      }
    } else {
      rv = NS_ERROR_OUT_OF_MEMORY;
    }
  }

  if (NS_FAILED(rv)) {
    delete topVal;
    delete rightVal;
    delete bottomVal;
    delete leftVal;
    delete val;

    return rv;
  }
  
  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetLineHeight(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  nscoord lineHeight;
  GetLineHeightCoord(lineHeight);
  val->SetAppUnits(lineHeight);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetVerticalAlign(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  SetValueToCoord(val, GetStyleTextReset()->mVerticalAlign,
                  &nsComputedDOMStyle::GetLineHeightCoord,
                  nsCSSProps::kVerticalAlignKTable);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetTextAlign(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsAFlatCString& align=
    nsCSSProps::ValueToKeyword(GetStyleText()->mTextAlign,
                               nsCSSProps::kTextAlignKTable);
  val->SetIdent(align);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetTextDecoration(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleTextReset* text = GetStyleTextReset();

  if (NS_STYLE_TEXT_DECORATION_NONE == text->mTextDecoration) {
    const nsAFlatCString& decoration=
      nsCSSKeywords::GetStringValue(eCSSKeyword_none);
    val->SetIdent(decoration);
  } else {
    nsAutoString decorationString;
    if (text->mTextDecoration & NS_STYLE_TEXT_DECORATION_UNDERLINE) {
      const nsAFlatCString& decoration=
        nsCSSProps::ValueToKeyword(NS_STYLE_TEXT_DECORATION_UNDERLINE,
                                   nsCSSProps::kTextDecorationKTable);
      decorationString.AppendWithConversion(decoration.get());
    }
    if (text->mTextDecoration & NS_STYLE_TEXT_DECORATION_OVERLINE) {
      if (!decorationString.IsEmpty()) {
        decorationString.Append(PRUnichar(' '));
      }
      const nsAFlatCString& decoration=
        nsCSSProps::ValueToKeyword(NS_STYLE_TEXT_DECORATION_OVERLINE,
                                   nsCSSProps::kTextDecorationKTable);
      decorationString.AppendWithConversion(decoration.get());
    }
    if (text->mTextDecoration & NS_STYLE_TEXT_DECORATION_LINE_THROUGH) {
      if (!decorationString.IsEmpty()) {
        decorationString.Append(PRUnichar(' '));
      }
      const nsAFlatCString& decoration=
        nsCSSProps::ValueToKeyword(NS_STYLE_TEXT_DECORATION_LINE_THROUGH,
                                   nsCSSProps::kTextDecorationKTable);
      decorationString.AppendWithConversion(decoration.get());
    }
    if (text->mTextDecoration & NS_STYLE_TEXT_DECORATION_BLINK) {
      if (!decorationString.IsEmpty()) {
        decorationString.Append(PRUnichar(' '));
      }
      const nsAFlatCString& decoration=
        nsCSSProps::ValueToKeyword(NS_STYLE_TEXT_DECORATION_BLINK,
                                   nsCSSProps::kTextDecorationKTable);
      decorationString.AppendWithConversion(decoration.get());
    }
    val->SetString(decorationString);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetTextIndent(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  SetValueToCoord(val, GetStyleText()->mTextIndent,
                  &nsComputedDOMStyle::GetCBContentWidth);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetTextTransform(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleText *text = GetStyleText();

  if (text->mTextTransform != NS_STYLE_TEXT_TRANSFORM_NONE) {
    const nsAFlatCString& textTransform =
      nsCSSProps::ValueToKeyword(text->mTextTransform,
                                 nsCSSProps::kTextTransformKTable);
    val->SetIdent(textTransform);
  } else {
    val->SetIdent(nsGkAtoms::none);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetLetterSpacing(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  SetValueToCoord(val, GetStyleText()->mLetterSpacing);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetWordSpacing(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  SetValueToCoord(val, GetStyleText()->mWordSpacing);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetWhiteSpace(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleText *text = GetStyleText();

  if (text->mWhiteSpace != NS_STYLE_WHITESPACE_NORMAL) {
    const nsAFlatCString& whiteSpace =
      nsCSSProps::ValueToKeyword(text->mWhiteSpace,
                                 nsCSSProps::kWhitespaceKTable);
    val->SetIdent(whiteSpace);
  } else {
    val->SetIdent(nsGkAtoms::normal);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetVisibility(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsAFlatCString& value=
    nsCSSProps::ValueToKeyword(GetStyleVisibility()->mVisible,
                               nsCSSProps::kVisibilityKTable);
  val->SetIdent(value);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetDirection(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsAFlatCString & direction =
    nsCSSProps::ValueToKeyword(GetStyleVisibility()->mDirection,
                               nsCSSProps::kDirectionKTable);
  val->SetIdent(direction);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetUnicodeBidi(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleTextReset *text = GetStyleTextReset();

  if (text->mUnicodeBidi != NS_STYLE_UNICODE_BIDI_NORMAL) {
    const nsAFlatCString& bidi =
      nsCSSProps::ValueToKeyword(text->mUnicodeBidi,
                                 nsCSSProps::kUnicodeBidiKTable);
    val->SetIdent(bidi);
  } else {
    val->SetIdent(nsGkAtoms::normal);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetCursor(nsIDOMCSSValue** aValue)
{
  nsDOMCSSValueList *valueList = GetROCSSValueList(PR_TRUE);
  NS_ENSURE_TRUE(valueList, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleUserInterface *ui = GetStyleUserInterface();

  for (nsCursorImage *item = ui->mCursorArray,
         *item_end = ui->mCursorArray + ui->mCursorArrayLength;
       item < item_end; ++item) {
    nsDOMCSSValueList *itemList = GetROCSSValueList(PR_FALSE);
    if (!itemList || !valueList->AppendCSSValue(itemList)) {
      delete itemList;
      delete valueList;
      return NS_ERROR_OUT_OF_MEMORY;
    }

    nsCOMPtr<nsIURI> uri;
    item->mImage->GetURI(getter_AddRefs(uri));

    nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
    if (!val || !itemList->AppendCSSValue(val)) {
      delete val;
      delete valueList;
      return NS_ERROR_OUT_OF_MEMORY;
    }
    val->SetURI(uri);

    if (item->mHaveHotspot) {
      nsROCSSPrimitiveValue *valX = GetROCSSPrimitiveValue();
      if (!valX || !itemList->AppendCSSValue(valX)) {
        delete valX;
        delete valueList;
        return NS_ERROR_OUT_OF_MEMORY;
      }
      nsROCSSPrimitiveValue *valY = GetROCSSPrimitiveValue();
      if (!valY || !itemList->AppendCSSValue(valY)) {
        delete valY;
        delete valueList;
        return NS_ERROR_OUT_OF_MEMORY;
      }

      valX->SetNumber(item->mHotspotX);
      valY->SetNumber(item->mHotspotY);
    }
  }

  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  if (!val) {
    delete valueList;
    return NS_ERROR_OUT_OF_MEMORY;
  }

  if (ui->mCursor == NS_STYLE_CURSOR_AUTO) {
    val->SetIdent(nsGkAtoms::_auto);
  } else {
    const nsAFlatCString& cursor =
      nsCSSProps::ValueToKeyword(ui->mCursor,
                                 nsCSSProps::kCursorKTable);
    val->SetIdent(cursor);
  }
  if (!valueList->AppendCSSValue(val)) {
    delete valueList;
    delete val;
    return NS_ERROR_OUT_OF_MEMORY;
  }

  return CallQueryInterface(valueList, aValue);
}

nsresult
nsComputedDOMStyle::GetAppearance(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsAFlatCString& appearanceIdent =
    nsCSSProps::ValueToKeyword(GetStyleDisplay()->mAppearance,
                               nsCSSProps::kAppearanceKTable);
  val->SetIdent(appearanceIdent);

  return CallQueryInterface(val, aValue);
}


nsresult
nsComputedDOMStyle::GetBoxAlign(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsAFlatCString& boxAlignIdent =
    nsCSSProps::ValueToKeyword(GetStyleXUL()->mBoxAlign,
                               nsCSSProps::kBoxAlignKTable);
  val->SetIdent(boxAlignIdent);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetBoxDirection(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsAFlatCString& boxDirectionIdent =
    nsCSSProps::ValueToKeyword(GetStyleXUL()->mBoxDirection,
                               nsCSSProps::kBoxDirectionKTable);
  val->SetIdent(boxDirectionIdent);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetBoxFlex(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  val->SetNumber(GetStyleXUL()->mBoxFlex);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetBoxOrdinalGroup(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  val->SetNumber(GetStyleXUL()->mBoxOrdinal);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetBoxOrient(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsAFlatCString& boxOrientIdent =
    nsCSSProps::ValueToKeyword(GetStyleXUL()->mBoxOrient,
                               nsCSSProps::kBoxOrientKTable);
  val->SetIdent(boxOrientIdent);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetBoxPack(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsAFlatCString& boxPackIdent =
    nsCSSProps::ValueToKeyword(GetStyleXUL()->mBoxPack,
                               nsCSSProps::kBoxPackKTable);
  val->SetIdent(boxPackIdent);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetBoxSizing(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsAFlatCString& boxSizingIdent =
    nsCSSProps::ValueToKeyword(GetStylePosition()->mBoxSizing,
                               nsCSSProps::kBoxSizingKTable);
  val->SetIdent(boxSizingIdent);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetFloatEdge(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsAFlatCString& floatEdgeIdent =
    nsCSSProps::ValueToKeyword(GetStyleBorder()->mFloatEdge,
                               nsCSSProps::kFloatEdgeKTable);
  val->SetIdent(floatEdgeIdent);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetForceBrokenImageIcon(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  val->SetNumber(GetStyleUIReset()->mForceBrokenImageIcon);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetIMEMode(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleUIReset *uiData = GetStyleUIReset();

  nsCSSKeyword keyword;
  if (uiData->mIMEMode == NS_STYLE_IME_MODE_AUTO) {
    keyword = eCSSKeyword_auto;
  } else if (uiData->mIMEMode == NS_STYLE_IME_MODE_NORMAL) {
    keyword = eCSSKeyword_normal;
  } else {
    keyword = nsCSSProps::ValueToKeywordEnum(uiData->mIMEMode,
                nsCSSProps::kIMEModeKTable);
  }
  val->SetIdent(nsCSSKeywords::GetStringValue(keyword));

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetUserFocus(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleUserInterface *uiData = GetStyleUserInterface();

  if (uiData->mUserFocus != NS_STYLE_USER_FOCUS_NONE) {
    if (uiData->mUserFocus == NS_STYLE_USER_FOCUS_NORMAL) {
      const nsAFlatCString& userFocusIdent =
              nsCSSKeywords::GetStringValue(eCSSKeyword_normal);
      val->SetIdent(userFocusIdent);
    } else {
      const nsAFlatCString& userFocusIdent =
        nsCSSProps::ValueToKeyword(uiData->mUserFocus,
                                   nsCSSProps::kUserFocusKTable);
      val->SetIdent(userFocusIdent);
    }
  } else {
    const nsAFlatCString& userFocusIdent =
            nsCSSKeywords::GetStringValue(eCSSKeyword_none);
    val->SetIdent(userFocusIdent);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetUserInput(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleUserInterface *uiData = GetStyleUserInterface();

  if (uiData->mUserInput != NS_STYLE_USER_INPUT_AUTO) {
    if (uiData->mUserInput == NS_STYLE_USER_INPUT_NONE) {
      const nsAFlatCString& userInputIdent =
        nsCSSKeywords::GetStringValue(eCSSKeyword_none);
      val->SetIdent(userInputIdent);
    } else {
      const nsAFlatCString& userInputIdent =
        nsCSSProps::ValueToKeyword(uiData->mUserInput,
                                   nsCSSProps::kUserInputKTable);
      val->SetIdent(userInputIdent);
    }
  } else {
    const nsAFlatCString& userInputIdent =
            nsCSSKeywords::GetStringValue(eCSSKeyword_auto);
    val->SetIdent(userInputIdent);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetUserModify(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsAFlatCString& userModifyIdent =
    nsCSSProps::ValueToKeyword(GetStyleUserInterface()->mUserModify,
                               nsCSSProps::kUserModifyKTable);
  val->SetIdent(userModifyIdent);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetUserSelect(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleUIReset *uiData = GetStyleUIReset();

  if (uiData->mUserSelect != NS_STYLE_USER_SELECT_AUTO) {
    if (uiData->mUserSelect == NS_STYLE_USER_SELECT_NONE) {
      const nsAFlatCString& userSelectIdent =
        nsCSSKeywords::GetStringValue(eCSSKeyword_none);
      val->SetIdent(userSelectIdent);
    } else {
      const nsAFlatCString& userSelectIdent =
        nsCSSProps::ValueToKeyword(uiData->mUserSelect,
                                   nsCSSProps::kUserSelectKTable);
      val->SetIdent(userSelectIdent);
    }
  } else {
    const nsAFlatCString& userSelectIdent =
            nsCSSKeywords::GetStringValue(eCSSKeyword_auto);
    val->SetIdent(userSelectIdent);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetDisplay(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleDisplay *displayData = GetStyleDisplay();

  if (displayData->mDisplay == NS_STYLE_DISPLAY_NONE) {
    val->SetIdent(nsGkAtoms::none);
  } else {
    const nsAFlatCString& display =
      nsCSSProps::ValueToKeyword(displayData->mDisplay,
                                 nsCSSProps::kDisplayKTable);
    val->SetIdent(display);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetPosition(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsAFlatCString& position =
    nsCSSProps::ValueToKeyword(GetStyleDisplay()->mPosition,
                               nsCSSProps::kPositionKTable);
  val->SetIdent(position);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetClip(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleDisplay* display = GetStyleDisplay();

  nsresult rv = NS_OK;
  nsROCSSPrimitiveValue *topVal = nsnull;
  nsROCSSPrimitiveValue *rightVal = nsnull;
  nsROCSSPrimitiveValue *bottomVal = nsnull;
  nsROCSSPrimitiveValue *leftVal = nsnull;
  if (display->mClipFlags == NS_STYLE_CLIP_AUTO ||
      display->mClipFlags == (NS_STYLE_CLIP_TOP_AUTO |
                              NS_STYLE_CLIP_RIGHT_AUTO |
                              NS_STYLE_CLIP_BOTTOM_AUTO |
                              NS_STYLE_CLIP_LEFT_AUTO)) {
    val->SetIdent(nsGkAtoms::_auto);
  } else {
    // create the cssvalues for the sides, stick them in the rect object
    topVal = GetROCSSPrimitiveValue();
    rightVal = GetROCSSPrimitiveValue();
    bottomVal = GetROCSSPrimitiveValue();
    leftVal = GetROCSSPrimitiveValue();
    if (topVal && rightVal && bottomVal && leftVal) {
      nsDOMCSSRect * domRect = new nsDOMCSSRect(topVal, rightVal,
                                                bottomVal, leftVal);
      if (domRect) {
        if (display->mClipFlags & NS_STYLE_CLIP_TOP_AUTO) {
          topVal->SetIdent(nsGkAtoms::_auto);
        } else {
          topVal->SetAppUnits(display->mClip.y);
        }
        
        if (display->mClipFlags & NS_STYLE_CLIP_RIGHT_AUTO) {
          rightVal->SetIdent(nsGkAtoms::_auto);
        } else {
          rightVal->SetAppUnits(display->mClip.width + display->mClip.x);
        }
        
        if (display->mClipFlags & NS_STYLE_CLIP_BOTTOM_AUTO) {
          bottomVal->SetIdent(nsGkAtoms::_auto);
        } else {
          bottomVal->SetAppUnits(display->mClip.height + display->mClip.y);
        }
        
        if (display->mClipFlags & NS_STYLE_CLIP_LEFT_AUTO) {
          leftVal->SetIdent(nsGkAtoms::_auto);
        } else {
          leftVal->SetAppUnits(display->mClip.x);
        }

        val->SetRect(domRect);
      } else {
        rv = NS_ERROR_OUT_OF_MEMORY;
      }
    } else {
      rv = NS_ERROR_OUT_OF_MEMORY;
    }
  }

  if (NS_FAILED(rv)) {
    delete topVal;
    delete rightVal;
    delete bottomVal;
    delete leftVal;
    delete val;

    return rv;
  }
  
  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetOverflow(nsIDOMCSSValue** aValue)
{
  const nsStyleDisplay* display = GetStyleDisplay();

  if (display->mOverflowX != display->mOverflowY) {
    // No value to return.  We can't express this combination of
    // values as a shorthand.
    *aValue = nsnull;
    return NS_OK;
  }
  
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  if (display->mOverflowX != NS_STYLE_OVERFLOW_AUTO) {
    const nsAFlatCString& overflow =
      nsCSSProps::ValueToKeyword(display->mOverflowX,
                                 nsCSSProps::kOverflowKTable);
    val->SetIdent(overflow);
  } else {
    val->SetIdent(nsGkAtoms::_auto);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetOverflowX(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleDisplay* display = GetStyleDisplay();

  if (display->mOverflowX != NS_STYLE_OVERFLOW_AUTO) {
    const nsAFlatCString& overflow =
      nsCSSProps::ValueToKeyword(display->mOverflowX,
                                 nsCSSProps::kOverflowSubKTable);
    val->SetIdent(overflow);
  } else {
    val->SetIdent(nsGkAtoms::_auto);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetOverflowY(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleDisplay* display = GetStyleDisplay();

  if (display->mOverflowY != NS_STYLE_OVERFLOW_AUTO) {
    const nsAFlatCString& overflow =
      nsCSSProps::ValueToKeyword(display->mOverflowY,
                                 nsCSSProps::kOverflowSubKTable);
    val->SetIdent(overflow);
  } else {
    val->SetIdent(nsGkAtoms::_auto);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetPageBreakAfter(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleDisplay *display = GetStyleDisplay();

  if (display->mBreakAfter) {
    val->SetIdent(nsGkAtoms::always);
  } else {
    val->SetIdent(nsGkAtoms::_auto);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetPageBreakBefore(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleDisplay *display = GetStyleDisplay();

  if (display->mBreakBefore) {
    val->SetIdent(nsGkAtoms::always);
  } else {
    val->SetIdent(nsGkAtoms::_auto);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetHeight(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  PRBool calcHeight = PR_FALSE;
  
  if (mInnerFrame) {
    calcHeight = PR_TRUE;

    const nsStyleDisplay* displayData = GetStyleDisplay();
    if (displayData->mDisplay == NS_STYLE_DISPLAY_INLINE &&
        !(mInnerFrame->IsFrameOfType(nsIFrame::eReplaced))) {
      calcHeight = PR_FALSE;
    }
  }

  if (calcHeight) {
    FlushPendingReflows();
  
    val->SetAppUnits(mInnerFrame->GetContentRect().height);
  } else {
    const nsStylePosition *positionData = GetStylePosition();

    nscoord minHeight =
      StyleCoordToNSCoord(positionData->mMinHeight,
                          &nsComputedDOMStyle::GetCBContentHeight, 0);

    nscoord maxHeight =
      StyleCoordToNSCoord(positionData->mMaxHeight,
                          &nsComputedDOMStyle::GetCBContentHeight,
                          nscoord_MAX);
    
    SetValueToCoord(val, positionData->mHeight, nsnull, nsnull,
                    minHeight, maxHeight);
  }
  
  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetWidth(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  PRBool calcWidth = PR_FALSE;

  if (mInnerFrame) {
    calcWidth = PR_TRUE;

    const nsStyleDisplay *displayData = GetStyleDisplay();
    if (displayData->mDisplay == NS_STYLE_DISPLAY_INLINE &&
        !(mInnerFrame->IsFrameOfType(nsIFrame::eReplaced))) {
      calcWidth = PR_FALSE;
    }
  }

  if (calcWidth) {
    FlushPendingReflows();

    val->SetAppUnits(mInnerFrame->GetContentRect().width);
  } else {
    const nsStylePosition *positionData = GetStylePosition();

    nscoord minWidth =
      StyleCoordToNSCoord(positionData->mMinWidth,
                          &nsComputedDOMStyle::GetCBContentWidth, 0);

    nscoord maxWidth =
      StyleCoordToNSCoord(positionData->mMaxWidth,
                          &nsComputedDOMStyle::GetCBContentWidth,
                          nscoord_MAX);
    
    SetValueToCoord(val, positionData->mWidth, nsnull,
                    nsCSSProps::kWidthKTable, minWidth, maxWidth);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetMaxHeight(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  SetValueToCoord(val, GetStylePosition()->mMaxHeight,
                  &nsComputedDOMStyle::GetCBContentHeight);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetMaxWidth(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  SetValueToCoord(val, GetStylePosition()->mMaxWidth,
                  &nsComputedDOMStyle::GetCBContentWidth,
                  nsCSSProps::kWidthKTable);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetMinHeight(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  SetValueToCoord(val, GetStylePosition()->mMinHeight,
                  &nsComputedDOMStyle::GetCBContentHeight);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetMinWidth(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  SetValueToCoord(val, GetStylePosition()->mMinWidth,
                  &nsComputedDOMStyle::GetCBContentWidth,
                  nsCSSProps::kWidthKTable);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetLeft(nsIDOMCSSValue** aValue)
{
  return GetOffsetWidthFor(NS_SIDE_LEFT, aValue);
}

nsresult
nsComputedDOMStyle::GetRight(nsIDOMCSSValue** aValue)
{
  return GetOffsetWidthFor(NS_SIDE_RIGHT, aValue);
}

nsresult
nsComputedDOMStyle::GetTop(nsIDOMCSSValue** aValue)
{
  return GetOffsetWidthFor(NS_SIDE_TOP, aValue);
}

nsROCSSPrimitiveValue*
nsComputedDOMStyle::GetROCSSPrimitiveValue()
{
  nsROCSSPrimitiveValue *primitiveValue = new nsROCSSPrimitiveValue(mAppUnitsPerInch);

  NS_ASSERTION(primitiveValue != 0, "ran out of memory");

  return primitiveValue;
}

nsDOMCSSValueList*
nsComputedDOMStyle::GetROCSSValueList(PRBool aCommaDelimited)
{
  nsDOMCSSValueList *valueList = new nsDOMCSSValueList(aCommaDelimited,
                                                       PR_TRUE);
  NS_ASSERTION(valueList != 0, "ran out of memory");

  return valueList;
}

nsresult
nsComputedDOMStyle::GetOffsetWidthFor(PRUint8 aSide, nsIDOMCSSValue** aValue)
{
  const nsStyleDisplay* display = GetStyleDisplay();

  FlushPendingReflows();

  nsresult rv = NS_OK;
  switch (display->mPosition) {
    case NS_STYLE_POSITION_STATIC:
      rv = GetStaticOffset(aSide, aValue);
      break;
    case NS_STYLE_POSITION_RELATIVE:
      rv = GetRelativeOffset(aSide, aValue);
      break;
    case NS_STYLE_POSITION_ABSOLUTE:
    case NS_STYLE_POSITION_FIXED:
      rv = GetAbsoluteOffset(aSide, aValue);
      break;
    default:
      NS_ERROR("Invalid position");
      break;
  }

  return rv;
}

nsresult
nsComputedDOMStyle::GetAbsoluteOffset(PRUint8 aSide, nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  nsIFrame* container = GetContainingBlockFor(mOuterFrame);
  if (container) {
    nsMargin margin = mOuterFrame->GetUsedMargin();
    nsMargin border = container->GetUsedBorder();
    nsMargin scrollbarSizes(0, 0, 0, 0);
    nsRect rect = mOuterFrame->GetRect();
    nsRect containerRect = container->GetRect();
      
    if (container->GetType() == nsGkAtoms::viewportFrame) {
      // For absolutely positioned frames scrollbars are taken into
      // account by virtue of getting a containing block that does
      // _not_ include the scrollbars.  For fixed positioned frames,
      // the containing block is the viewport, which _does_ include
      // scrollbars.  We have to do some extra work.
      // the first child in the default frame list is what we want
      nsIFrame* scrollingChild = container->GetFirstChild(nsnull);
      nsCOMPtr<nsIScrollableFrame> scrollFrame =
        do_QueryInterface(scrollingChild);
      if (scrollFrame) {
        scrollbarSizes = scrollFrame->GetActualScrollbarSizes();
      }
    }

    nscoord offset = 0;
    switch (aSide) {
      case NS_SIDE_TOP:
        offset = rect.y - margin.top - border.top - scrollbarSizes.top;

        break;
      case NS_SIDE_RIGHT:
        offset = containerRect.width - rect.width -
          rect.x - margin.right - border.right - scrollbarSizes.right;

        break;
      case NS_SIDE_BOTTOM:
        offset = containerRect.height - rect.height -
          rect.y - margin.bottom - border.bottom - scrollbarSizes.bottom;

        break;
      case NS_SIDE_LEFT:
        offset = rect.x - margin.left - border.left - scrollbarSizes.left;

        break;
      default:
        NS_ERROR("Invalid side");
        break;
    }
    val->SetAppUnits(offset);
  } else {
    // XXX no frame.  This property makes no sense
    val->SetAppUnits(0);
  }

  return CallQueryInterface(val, aValue);
}


#if (NS_SIDE_TOP == 0) && (NS_SIDE_RIGHT == 1) && (NS_SIDE_BOTTOM == 2) && (NS_SIDE_LEFT == 3)
#define NS_OPPOSITE_SIDE(s_) (((s_) + 2) & 3)
#else
#error "Somebody changed the side constants."
#endif

nsresult
nsComputedDOMStyle::GetRelativeOffset(PRUint8 aSide, nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStylePosition* positionData = GetStylePosition();
  nsStyleCoord coord;
  PRInt32 sign = 1;
  positionData->mOffset.Get(aSide, coord);

  NS_ASSERTION(coord.GetUnit() == eStyleUnit_Coord ||
               coord.GetUnit() == eStyleUnit_Percent ||
               coord.GetUnit() == eStyleUnit_Chars ||
               coord.GetUnit() == eStyleUnit_Auto,
               "Unexpected unit");
  
  if (coord.GetUnit() == eStyleUnit_Auto) {
    positionData->mOffset.Get(NS_OPPOSITE_SIDE(aSide), coord);
    sign = -1;
  }
  PercentageBaseGetter baseGetter;
  if (aSide == NS_SIDE_LEFT || aSide == NS_SIDE_RIGHT) {
    baseGetter = &nsComputedDOMStyle::GetCBContentWidth;
  } else {
    baseGetter = &nsComputedDOMStyle::GetCBContentHeight;
  }

  val->SetAppUnits(sign * StyleCoordToNSCoord(coord, baseGetter, 0));

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetStaticOffset(PRUint8 aSide, nsIDOMCSSValue** aValue)

{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  nsStyleCoord coord;
  SetValueToCoord(val, GetStylePosition()->mOffset.Get(aSide, coord));
  
  return CallQueryInterface(val, aValue);
}

void
nsComputedDOMStyle::FlushPendingReflows()
{
  // Flush all pending notifications so that our frames are up to date
  nsCOMPtr<nsIDocument> document = mContent->GetDocument();
  if (document) {
    document->FlushPendingNotifications(Flush_Layout);
  }
}

nsresult
nsComputedDOMStyle::GetPaddingWidthFor(PRUint8 aSide, nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  if (!mInnerFrame) {
    nsStyleCoord c;
    SetValueToCoord(val, GetStylePadding()->mPadding.Get(aSide, c));
  } else {
    FlushPendingReflows();
  
    val->SetAppUnits(mInnerFrame->GetUsedPadding().side(aSide));
  }

  return CallQueryInterface(val, aValue);
}

PRBool
nsComputedDOMStyle::GetLineHeightCoord(nscoord& aCoord)
{
  // Get a rendering context
  nsCOMPtr<nsIRenderingContext> cx;
  nsIFrame* frame = mPresShell->FrameManager()->GetRootFrame();
  if (frame) {
    mPresShell->CreateRenderingContext(frame, getter_AddRefs(cx));
  }
  if (!cx) {
    // Give up
    aCoord = 0;
    return PR_FALSE;
  }
  
  aCoord = nsHTMLReflowState::CalcLineHeight(cx, mStyleContextHolder);
  
  // CalcLineHeight uses font->mFont.size, but we want to use
  // font->mSize as the font size.  Adjust for that.  Also adjust for
  // the text zoom, if any.
  const nsStyleFont* font = GetStyleFont();
  aCoord = NSToCoordRound((float(aCoord) *
                           (float(font->mSize) / float(font->mFont.size))) /
                          mPresShell->GetPresContext()->TextZoom());
  
  return PR_TRUE;
}

nsresult
nsComputedDOMStyle::GetBorderColorsFor(PRUint8 aSide, nsIDOMCSSValue** aValue)
{
  const nsStyleBorder *border = GetStyleBorder();

  if (border->mBorderColors) {
    nsBorderColors* borderColors = border->mBorderColors[aSide];
    if (borderColors) {
      nsDOMCSSValueList *valueList = GetROCSSValueList(PR_FALSE);
      NS_ENSURE_TRUE(valueList, NS_ERROR_OUT_OF_MEMORY);

      do {
        nsROCSSPrimitiveValue *primitive = GetROCSSPrimitiveValue();
        if (!primitive) {
          delete valueList;

          return NS_ERROR_OUT_OF_MEMORY;
        }
        if (borderColors->mTransparent) {
          primitive->SetIdent(nsGkAtoms::transparent);
        } else {
          nsresult rv = SetToRGBAColor(primitive, borderColors->mColor);
          if (NS_FAILED(rv)) {
            delete valueList;
            delete primitive;
            return rv;
          }
        }

        PRBool success = valueList->AppendCSSValue(primitive);
        if (!success) {
          delete valueList;
          delete primitive;

          return NS_ERROR_OUT_OF_MEMORY;
        }
        borderColors = borderColors->mNext;
      } while (borderColors);

      return CallQueryInterface(valueList, aValue);
    }
  }

  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  val->SetIdent(nsGkAtoms::none);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetBorderRadiusFor(PRUint8 aSide, nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  nsStyleCoord coord;
  SetValueToCoord(val, GetStyleBorder()->mBorderRadius.Get(aSide, coord),
                  &nsComputedDOMStyle::GetFrameBorderRectWidth);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetBorderWidthFor(PRUint8 aSide, nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  nscoord width;
  if (mInnerFrame) {
    FlushPendingReflows();
    width = mInnerFrame->GetUsedBorder().side(aSide);
  } else {
    width = GetStyleBorder()->GetBorderWidth(aSide);
  }
  val->SetAppUnits(width);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetBorderColorFor(PRUint8 aSide, nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  nscolor color; 
  PRBool transparent;
  PRBool foreground;
  GetStyleBorder()->GetBorderColor(aSide, color, transparent, foreground);
  if (transparent) {
    val->SetIdent(nsGkAtoms::transparent);
  } else {
    if (foreground) {
      const nsStyleColor* colorStruct = GetStyleColor();
      color = colorStruct->mColor;
    }
    // XXX else?

    nsresult rv = SetToRGBAColor(val, color);
    if (NS_FAILED(rv)) {
      delete val;
      return rv;
    }
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetMarginWidthFor(PRUint8 aSide, nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  if (!mInnerFrame) {
    nsStyleCoord c;
    SetValueToCoord(val, GetStyleMargin()->mMargin.Get(aSide, c));
  } else {
    FlushPendingReflows();

    val->SetAppUnits(mInnerFrame->GetUsedMargin().side(aSide));
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetBorderStyleFor(PRUint8 aSide, nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  PRUint8 borderStyle = GetStyleBorder()->GetBorderStyle(aSide);

  if (borderStyle != NS_STYLE_BORDER_STYLE_NONE) {
    const nsAFlatCString& style=
      nsCSSProps::ValueToKeyword(borderStyle,
                                 nsCSSProps::kBorderStyleKTable);
    val->SetIdent(style);
  } else {
    val->SetIdent(nsGkAtoms::none);
  }

  return CallQueryInterface(val, aValue);
}

void
nsComputedDOMStyle::SetValueToCoord(nsROCSSPrimitiveValue* aValue,
                                    const nsStyleCoord& aCoord,
                                    PercentageBaseGetter aPercentageBaseGetter,
                                    const PRInt32 aTable[],
                                    nscoord aMinAppUnits,
                                    nscoord aMaxAppUnits)
{
  NS_PRECONDITION(aValue, "Must have a value to work with");
  
  switch (aCoord.GetUnit()) {
    case eStyleUnit_Normal:
      aValue->SetIdent(nsGkAtoms::normal);
      break;
      
    case eStyleUnit_Auto:
      aValue->SetIdent(nsGkAtoms::_auto);
      break;
      
    case eStyleUnit_Percent:
      {
        nscoord percentageBase;
        if (aPercentageBaseGetter &&
            (this->*aPercentageBaseGetter)(percentageBase)) {
          nscoord val = nscoord(aCoord.GetPercentValue() * percentageBase);
          aValue->SetAppUnits(PR_MAX(aMinAppUnits, PR_MIN(val, aMaxAppUnits)));
        } else {
          aValue->SetPercent(aCoord.GetPercentValue());
        }
      }
      break;
      
    case eStyleUnit_Factor:
      aValue->SetNumber(aCoord.GetFactorValue());
      break;
      
    case eStyleUnit_Coord:
      {
        nscoord val = aCoord.GetCoordValue();
        aValue->SetAppUnits(PR_MAX(aMinAppUnits, PR_MIN(val, aMaxAppUnits)));
      }
      break;
      
    case eStyleUnit_Integer:
      aValue->SetNumber(aCoord.GetIntValue());
      break;
      
    case eStyleUnit_Enumerated:
      NS_ASSERTION(aTable, "Must have table to handle this case");
      aValue->SetIdent(nsCSSProps::ValueToKeyword(aCoord.GetIntValue(),
                                                  aTable));
      break;
      
    case eStyleUnit_Chars: {
      // Get a rendering context
      nsCOMPtr<nsIRenderingContext> cx;
      nsIFrame* frame = mPresShell->FrameManager()->GetRootFrame();
      if (frame) {
        mPresShell->CreateRenderingContext(frame, getter_AddRefs(cx));
      }
      if (cx) {
        nscoord val =
          nsLayoutUtils::CharsToCoord(aCoord, cx, mStyleContextHolder);
        aValue->SetAppUnits(PR_MAX(aMinAppUnits, PR_MIN(val, aMaxAppUnits)));
      } else {
        // Oh, well.  Give up.
        aValue->SetAppUnits(0);
      }
      break;
    }

    case eStyleUnit_None:
      aValue->SetIdent(nsGkAtoms::none);
      break;
      
    default:
      NS_ERROR("Can't handle this unit");
      break;
  }
}

nscoord
nsComputedDOMStyle::StyleCoordToNSCoord(const nsStyleCoord& aCoord,
                                        PercentageBaseGetter aPercentageBaseGetter,
                                        nscoord aDefaultValue)
{
  NS_PRECONDITION(aPercentageBaseGetter, "Must have a percentage base getter");
  switch (aCoord.GetUnit()) {
    case eStyleUnit_Coord:
      return aCoord.GetCoordValue();
    case eStyleUnit_Chars:
      {
        // Get a rendering context
        nsCOMPtr<nsIRenderingContext> cx;
        nsIFrame* frame = mPresShell->FrameManager()->GetRootFrame();
        if (frame) {
          mPresShell->CreateRenderingContext(frame, getter_AddRefs(cx));
        }
        if (!cx) {
          // Return the default value, I guess
          break;
        }

        return nsLayoutUtils::CharsToCoord(aCoord, cx, mStyleContextHolder);
      }
    case eStyleUnit_Percent:
      {
        nscoord percentageBase;
        if ((this->*aPercentageBaseGetter)(percentageBase)) {
          return nscoord(aCoord.GetPercentValue() * percentageBase);
        }
      }
      // Fall through to returning aDefaultValue if we have no percentage base.
    default:
      break;
  }
      
  return aDefaultValue;
}

PRBool
nsComputedDOMStyle::GetCBContentWidth(nscoord& aWidth)
{
  if (!mOuterFrame) {
    return PR_FALSE;
  }

  nsIFrame* container = GetContainingBlockFor(mOuterFrame);
  if (!container) {
    return PR_FALSE;
  }

  FlushPendingReflows();

  aWidth = container->GetContentRect().width;
  return PR_TRUE;
}

PRBool
nsComputedDOMStyle::GetCBContentHeight(nscoord& aHeight)
{
  if (!mOuterFrame) {
    return PR_FALSE;
  }

  nsIFrame* container = GetContainingBlockFor(mOuterFrame);
  if (!container) {
    return PR_FALSE;
  }

  FlushPendingReflows();

  aHeight = container->GetContentRect().height;
  return PR_TRUE;
}

PRBool
nsComputedDOMStyle::GetFrameBorderRectWidth(nscoord& aWidth)
{
  if (!mInnerFrame) {
    return PR_FALSE;
  }

  FlushPendingReflows();

  aWidth = mInnerFrame->GetSize().width;
  return PR_TRUE;
}

#ifdef MOZ_SVG

nsresult
nsComputedDOMStyle::GetSVGPaintFor(PRBool aFill,
                                   nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleSVG* svg = GetStyleSVG();
  const nsStyleSVGPaint* paint = nsnull;

  if (aFill)
    paint = &svg->mFill;
  else
    paint = &svg->mStroke;

  nsAutoString paintString;

  switch (paint->mType) {
  case eStyleSVGPaintType_None:
  {
    val->SetIdent(nsGkAtoms::none);
    break;
  }
  case eStyleSVGPaintType_Color:
  {
    nsresult rv = SetToRGBAColor(val, paint->mPaint.mColor);
    if (NS_FAILED(rv)) {
      delete val;
      return rv;
    }
    break;
  }
  case eStyleSVGPaintType_Server:
  {
    nsDOMCSSValueList *valueList = GetROCSSValueList(PR_FALSE);
    NS_ENSURE_TRUE(valueList, NS_ERROR_OUT_OF_MEMORY);

    if (!valueList->AppendCSSValue(val)) {
      delete valueList;
      delete val;
      return NS_ERROR_OUT_OF_MEMORY;
    }

    nsROCSSPrimitiveValue* fallback = GetROCSSPrimitiveValue();
    if (!fallback || !valueList->AppendCSSValue(fallback)) {
      delete valueList;
      delete fallback;
      return NS_ERROR_OUT_OF_MEMORY;
    }

    val->SetURI(paint->mPaint.mPaintServer);
    nsresult rv = SetToRGBAColor(fallback, paint->mFallbackColor);
    if (NS_FAILED(rv)) {
      delete valueList;
      return rv;
    }

    return CallQueryInterface(valueList, aValue);
  }
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetFill(nsIDOMCSSValue** aValue)
{
  return GetSVGPaintFor(PR_TRUE, aValue);
}

nsresult
nsComputedDOMStyle::GetStroke(nsIDOMCSSValue** aValue)
{
  return GetSVGPaintFor(PR_FALSE, aValue);
}

nsresult
nsComputedDOMStyle::GetMarkerEnd(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleSVG* svg = GetStyleSVG();

  if (svg->mMarkerEnd)
    val->SetURI(svg->mMarkerEnd);
  else
    val->SetIdent(nsGkAtoms::none);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetMarkerMid(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleSVG* svg = GetStyleSVG();

  if (svg->mMarkerMid)
    val->SetURI(svg->mMarkerMid);
  else
    val->SetIdent(nsGkAtoms::none);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetMarkerStart(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleSVG* svg = GetStyleSVG();

  if (svg->mMarkerStart)
    val->SetURI(svg->mMarkerStart);
  else
    val->SetIdent(nsGkAtoms::none);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetStrokeDasharray(nsIDOMCSSValue** aValue)
{
  const nsStyleSVG* svg = GetStyleSVG();

  if (!svg->mStrokeDasharrayLength || !svg->mStrokeDasharray) {
    nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
    NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);
    val->SetIdent(nsGkAtoms::none);
    return CallQueryInterface(val, aValue);
  }

  nsDOMCSSValueList *valueList = GetROCSSValueList(PR_TRUE);
  NS_ENSURE_TRUE(valueList, NS_ERROR_OUT_OF_MEMORY);

  for (PRUint32 i = 0; i < svg->mStrokeDasharrayLength; i++) {
    nsROCSSPrimitiveValue* dash = GetROCSSPrimitiveValue();
    if (!dash || !valueList->AppendCSSValue(dash)) {
      delete valueList;
      delete dash;
      return NS_ERROR_OUT_OF_MEMORY;
    }

    SetValueToCoord(dash, svg->mStrokeDasharray[i]);
  }

  return CallQueryInterface(valueList, aValue);
}

nsresult
nsComputedDOMStyle::GetStrokeDashoffset(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  SetValueToCoord(val, GetStyleSVG()->mStrokeDashoffset);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetStrokeWidth(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  SetValueToCoord(val, GetStyleSVG()->mStrokeWidth);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetFillOpacity(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  val->SetNumber(GetStyleSVG()->mFillOpacity);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetFloodOpacity(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  val->SetNumber(GetStyleSVGReset()->mFloodOpacity);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetStopOpacity(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  val->SetNumber(GetStyleSVGReset()->mStopOpacity);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetStrokeMiterlimit(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  val->SetNumber(GetStyleSVG()->mStrokeMiterlimit);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetStrokeOpacity(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  val->SetNumber(GetStyleSVG()->mStrokeOpacity);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetClipRule(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsAFlatCString& keyword =
    nsCSSProps::ValueToKeyword(GetStyleSVG()->mClipRule,
                               nsCSSProps::kFillRuleKTable);
  val->SetIdent(keyword);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetFillRule(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsAFlatCString& keyword =
    nsCSSProps::ValueToKeyword(GetStyleSVG()->mFillRule,
                               nsCSSProps::kFillRuleKTable);
  val->SetIdent(keyword);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetStrokeLinecap(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsAFlatCString& keyword =
    nsCSSProps::ValueToKeyword(GetStyleSVG()->mStrokeLinecap,
                               nsCSSProps::kStrokeLinecapKTable);
  val->SetIdent(keyword);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetStrokeLinejoin(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsAFlatCString& keyword =
    nsCSSProps::ValueToKeyword(GetStyleSVG()->mStrokeLinejoin,
                               nsCSSProps::kStrokeLinejoinKTable);
  val->SetIdent(keyword);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetTextAnchor(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsAFlatCString& keyword =
    nsCSSProps::ValueToKeyword(GetStyleSVG()->mTextAnchor,
                               nsCSSProps::kTextAnchorKTable);
  val->SetIdent(keyword);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetColorInterpolation(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleSVG* svg = GetStyleSVG();

  if (svg->mColorInterpolation != NS_STYLE_COLOR_INTERPOLATION_AUTO) {
    const nsAFlatCString& keyword =
      nsCSSProps::ValueToKeyword(svg->mColorInterpolation,
                                 nsCSSProps::kColorInterpolationKTable);
    val->SetIdent(keyword);
  } else {
    val->SetIdent(nsGkAtoms::_auto);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetColorInterpolationFilters(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleSVG* svg = GetStyleSVG();

  if (svg->mColorInterpolationFilters != NS_STYLE_COLOR_INTERPOLATION_AUTO) {
    const nsAFlatCString& keyword =
      nsCSSProps::ValueToKeyword(svg->mColorInterpolationFilters,
                                 nsCSSProps::kColorInterpolationKTable);
    val->SetIdent(keyword);
  } else {
    val->SetIdent(nsGkAtoms::_auto);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetDominantBaseline(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleSVGReset* svg = GetStyleSVGReset();

  if (svg->mDominantBaseline != NS_STYLE_DOMINANT_BASELINE_AUTO) {
    const nsAFlatCString& keyword =
      nsCSSProps::ValueToKeyword(svg->mDominantBaseline,
                                 nsCSSProps::kDominantBaselineKTable);
    val->SetIdent(keyword);
  } else {
    val->SetIdent(nsGkAtoms::_auto);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetPointerEvents(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleSVG* svg = GetStyleSVG();

  if (svg->mPointerEvents != NS_STYLE_POINTER_EVENTS_NONE) {
    const nsAFlatCString& keyword =
      nsCSSProps::ValueToKeyword(svg->mPointerEvents,
                                 nsCSSProps::kPointerEventsKTable);
    val->SetIdent(keyword);
  } else {
    val->SetIdent(nsGkAtoms::none);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetShapeRendering(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleSVG* svg = GetStyleSVG();

  if (svg->mShapeRendering != NS_STYLE_SHAPE_RENDERING_AUTO) {
    const nsAFlatCString& keyword =
      nsCSSProps::ValueToKeyword(svg->mShapeRendering,
                                 nsCSSProps::kShapeRenderingKTable);
    val->SetIdent(keyword);
  } else {
    val->SetIdent(nsGkAtoms::_auto);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetTextRendering(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleSVG* svg = GetStyleSVG();

  if (svg->mTextRendering != NS_STYLE_TEXT_RENDERING_AUTO) {
    const nsAFlatCString& keyword =
      nsCSSProps::ValueToKeyword(svg->mTextRendering,
                                 nsCSSProps::kTextRenderingKTable);
    val->SetIdent(keyword);
  } else {
    val->SetIdent(nsGkAtoms::_auto);
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetFloodColor(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  nsresult rv = SetToRGBAColor(val, GetStyleSVGReset()->mFloodColor);
  if (NS_FAILED(rv)) {
    delete val;
    return rv;
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetLightingColor(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  nsresult rv = SetToRGBAColor(val, GetStyleSVGReset()->mLightingColor);
  if (NS_FAILED(rv)) {
    delete val;
    return rv;
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetStopColor(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue *val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  nsresult rv = SetToRGBAColor(val, GetStyleSVGReset()->mStopColor);
  if (NS_FAILED(rv)) {
    delete val;
    return rv;
  }

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetClipPath(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleSVGReset* svg = GetStyleSVGReset();

  if (svg->mClipPath)
    val->SetURI(svg->mClipPath);
  else
    val->SetIdent(nsGkAtoms::none);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetFilter(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleSVGReset* svg = GetStyleSVGReset();

  if (svg->mFilter)
    val->SetURI(svg->mFilter);
  else
    val->SetIdent(nsGkAtoms::none);

  return CallQueryInterface(val, aValue);
}

nsresult
nsComputedDOMStyle::GetMask(nsIDOMCSSValue** aValue)
{
  nsROCSSPrimitiveValue* val = GetROCSSPrimitiveValue();
  NS_ENSURE_TRUE(val, NS_ERROR_OUT_OF_MEMORY);

  const nsStyleSVGReset* svg = GetStyleSVGReset();

  if (svg->mMask)
    val->SetURI(svg->mMask);
  else
    val->SetIdent(nsGkAtoms::none);

  return CallQueryInterface(val, aValue);
}

#endif // MOZ_SVG


#define COMPUTED_STYLE_MAP_ENTRY(_prop, _method)              \
  { eCSSProperty_##_prop, &nsComputedDOMStyle::Get##_method }

const nsComputedDOMStyle::ComputedStyleMapEntry*
nsComputedDOMStyle::GetQueryablePropertyMap(PRUint32* aLength)
{
  /* ******************************************************************* *\
   * Properties below are listed in alphabetical order.                  *
   * Please keep them that way.                                          *
   *                                                                     *
   * Properties commented out with // are not yet implemented            *
   * Properties commented out with //// are shorthands and not queryable *
  \* ******************************************************************* */
  static
#ifndef XP_MACOSX
    // XXX If this actually fixes the bustage, replace this with an
    // autoconf test.
  const
#endif
  ComputedStyleMapEntry map[] = {
    /* ****************************** *\
     * Implementations of CSS2 styles *
    \* ****************************** */

    // COMPUTED_STYLE_MAP_ENTRY(azimuth,                    Azimuth),
    //// COMPUTED_STYLE_MAP_ENTRY(background,               Background),
    COMPUTED_STYLE_MAP_ENTRY(background_attachment,         BackgroundAttachment),
    COMPUTED_STYLE_MAP_ENTRY(background_color,              BackgroundColor),
    COMPUTED_STYLE_MAP_ENTRY(background_image,              BackgroundImage),
    COMPUTED_STYLE_MAP_ENTRY(background_position,           BackgroundPosition),
    COMPUTED_STYLE_MAP_ENTRY(background_repeat,             BackgroundRepeat),
    //// COMPUTED_STYLE_MAP_ENTRY(border,                   Border),
    //// COMPUTED_STYLE_MAP_ENTRY(border_bottom,            BorderBottom),
    COMPUTED_STYLE_MAP_ENTRY(border_bottom_color,           BorderBottomColor),
    COMPUTED_STYLE_MAP_ENTRY(border_bottom_style,           BorderBottomStyle),
    COMPUTED_STYLE_MAP_ENTRY(border_bottom_width,           BorderBottomWidth),
    COMPUTED_STYLE_MAP_ENTRY(border_collapse,               BorderCollapse),
    //// COMPUTED_STYLE_MAP_ENTRY(border_color,             BorderColor),
    //// COMPUTED_STYLE_MAP_ENTRY(border_left,              BorderLeft),
    COMPUTED_STYLE_MAP_ENTRY(border_left_color,             BorderLeftColor),
    COMPUTED_STYLE_MAP_ENTRY(border_left_style,             BorderLeftStyle),
    COMPUTED_STYLE_MAP_ENTRY(border_left_width,             BorderLeftWidth),
    //// COMPUTED_STYLE_MAP_ENTRY(border_right,             BorderRight),
    COMPUTED_STYLE_MAP_ENTRY(border_right_color,            BorderRightColor),
    COMPUTED_STYLE_MAP_ENTRY(border_right_style,            BorderRightStyle),
    COMPUTED_STYLE_MAP_ENTRY(border_right_width,            BorderRightWidth),
    COMPUTED_STYLE_MAP_ENTRY(border_spacing,                BorderSpacing),
    //// COMPUTED_STYLE_MAP_ENTRY(border_style,             BorderStyle),
    //// COMPUTED_STYLE_MAP_ENTRY(border_top,               BorderTop),
    COMPUTED_STYLE_MAP_ENTRY(border_top_color,              BorderTopColor),
    COMPUTED_STYLE_MAP_ENTRY(border_top_style,              BorderTopStyle),
    COMPUTED_STYLE_MAP_ENTRY(border_top_width,              BorderTopWidth),
    //// COMPUTED_STYLE_MAP_ENTRY(border_width,             BorderWidth),
    COMPUTED_STYLE_MAP_ENTRY(bottom,                        Bottom),
    COMPUTED_STYLE_MAP_ENTRY(caption_side,                  CaptionSide),
    COMPUTED_STYLE_MAP_ENTRY(clear,                         Clear),
    COMPUTED_STYLE_MAP_ENTRY(clip,                          Clip),
    COMPUTED_STYLE_MAP_ENTRY(color,                         Color),
    COMPUTED_STYLE_MAP_ENTRY(content,                       Content),
    COMPUTED_STYLE_MAP_ENTRY(counter_increment,             CounterIncrement),
    COMPUTED_STYLE_MAP_ENTRY(counter_reset,                 CounterReset),
    //// COMPUTED_STYLE_MAP_ENTRY(cue,                      Cue),
    // COMPUTED_STYLE_MAP_ENTRY(cue_after,                  CueAfter),
    // COMPUTED_STYLE_MAP_ENTRY(cue_before,                 CueBefore),
    COMPUTED_STYLE_MAP_ENTRY(cursor,                        Cursor),
    COMPUTED_STYLE_MAP_ENTRY(direction,                     Direction),
    COMPUTED_STYLE_MAP_ENTRY(display,                       Display),
    // COMPUTED_STYLE_MAP_ENTRY(elevation,                  Elevation),
    COMPUTED_STYLE_MAP_ENTRY(empty_cells,                   EmptyCells),
    COMPUTED_STYLE_MAP_ENTRY(float,                         CssFloat),
    //// COMPUTED_STYLE_MAP_ENTRY(font,                     Font),
    COMPUTED_STYLE_MAP_ENTRY(font_family,                   FontFamily),
    COMPUTED_STYLE_MAP_ENTRY(font_size,                     FontSize),
    COMPUTED_STYLE_MAP_ENTRY(font_size_adjust,              FontSizeAdjust),
    // COMPUTED_STYLE_MAP_ENTRY(font_stretch,               FontStretch),
    COMPUTED_STYLE_MAP_ENTRY(font_style,                    FontStyle),
    COMPUTED_STYLE_MAP_ENTRY(font_variant,                  FontVariant),
    COMPUTED_STYLE_MAP_ENTRY(font_weight,                   FontWeight),
    COMPUTED_STYLE_MAP_ENTRY(height,                        Height),
    COMPUTED_STYLE_MAP_ENTRY(left,                          Left),
    COMPUTED_STYLE_MAP_ENTRY(letter_spacing,                LetterSpacing),
    COMPUTED_STYLE_MAP_ENTRY(line_height,                   LineHeight),
    //// COMPUTED_STYLE_MAP_ENTRY(list_style,               ListStyle),
    COMPUTED_STYLE_MAP_ENTRY(list_style_image,              ListStyleImage),
    COMPUTED_STYLE_MAP_ENTRY(list_style_position,           ListStylePosition),
    COMPUTED_STYLE_MAP_ENTRY(list_style_type,               ListStyleType),
    //// COMPUTED_STYLE_MAP_ENTRY(margin,                   Margin),
    COMPUTED_STYLE_MAP_ENTRY(margin_bottom,                 MarginBottomWidth),
    COMPUTED_STYLE_MAP_ENTRY(margin_left,                   MarginLeftWidth),
    COMPUTED_STYLE_MAP_ENTRY(margin_right,                  MarginRightWidth),
    COMPUTED_STYLE_MAP_ENTRY(margin_top,                    MarginTopWidth),
    COMPUTED_STYLE_MAP_ENTRY(marker_offset,                 MarkerOffset),
    // COMPUTED_STYLE_MAP_ENTRY(marks,                      Marks),
    COMPUTED_STYLE_MAP_ENTRY(max_height,                    MaxHeight),
    COMPUTED_STYLE_MAP_ENTRY(max_width,                     MaxWidth),
    COMPUTED_STYLE_MAP_ENTRY(min_height,                    MinHeight),
    COMPUTED_STYLE_MAP_ENTRY(min_width,                     MinWidth),
    COMPUTED_STYLE_MAP_ENTRY(ime_mode,                      IMEMode),
    COMPUTED_STYLE_MAP_ENTRY(opacity,                       Opacity),
    // COMPUTED_STYLE_MAP_ENTRY(orphans,                    Orphans),
    //// COMPUTED_STYLE_MAP_ENTRY(outline,                  Outline),
    COMPUTED_STYLE_MAP_ENTRY(outline_color,                 OutlineColor),
    COMPUTED_STYLE_MAP_ENTRY(outline_style,                 OutlineStyle),
    COMPUTED_STYLE_MAP_ENTRY(outline_width,                 OutlineWidth),
    COMPUTED_STYLE_MAP_ENTRY(outline_offset,                OutlineOffset),
    COMPUTED_STYLE_MAP_ENTRY(overflow,                      Overflow),
    COMPUTED_STYLE_MAP_ENTRY(overflow_x,                    OverflowX),
    COMPUTED_STYLE_MAP_ENTRY(overflow_y,                    OverflowY),
    //// COMPUTED_STYLE_MAP_ENTRY(padding,                  Padding),
    COMPUTED_STYLE_MAP_ENTRY(padding_bottom,                PaddingBottom),
    COMPUTED_STYLE_MAP_ENTRY(padding_left,                  PaddingLeft),
    COMPUTED_STYLE_MAP_ENTRY(padding_right,                 PaddingRight),
    COMPUTED_STYLE_MAP_ENTRY(padding_top,                   PaddingTop),
    // COMPUTED_STYLE_MAP_ENTRY(page,                       Page),
    COMPUTED_STYLE_MAP_ENTRY(page_break_after,              PageBreakAfter),
    COMPUTED_STYLE_MAP_ENTRY(page_break_before,             PageBreakBefore),
    // COMPUTED_STYLE_MAP_ENTRY(page_break_inside,          PageBreakInside),
    //// COMPUTED_STYLE_MAP_ENTRY(pause,                    Pause),
    // COMPUTED_STYLE_MAP_ENTRY(pause_after,                PauseAfter),
    // COMPUTED_STYLE_MAP_ENTRY(pause_before,               PauseBefore),
    // COMPUTED_STYLE_MAP_ENTRY(pitch,                      Pitch),
    // COMPUTED_STYLE_MAP_ENTRY(pitch_range,                PitchRange),
    COMPUTED_STYLE_MAP_ENTRY(position,                      Position),
    COMPUTED_STYLE_MAP_ENTRY(quotes,                        Quotes),
    // COMPUTED_STYLE_MAP_ENTRY(richness,                   Richness),
    COMPUTED_STYLE_MAP_ENTRY(right,                         Right),
    //// COMPUTED_STYLE_MAP_ENTRY(size,                     Size),
    // COMPUTED_STYLE_MAP_ENTRY(speak,                      Speak),
    // COMPUTED_STYLE_MAP_ENTRY(speak_header,               SpeakHeader),
    // COMPUTED_STYLE_MAP_ENTRY(speak_numeral,              SpeakNumeral),
    // COMPUTED_STYLE_MAP_ENTRY(speak_punctuation,          SpeakPunctuation),
    // COMPUTED_STYLE_MAP_ENTRY(speech_rate,                SpeechRate),
    // COMPUTED_STYLE_MAP_ENTRY(stress,                     Stress),
    COMPUTED_STYLE_MAP_ENTRY(table_layout,                  TableLayout),
    COMPUTED_STYLE_MAP_ENTRY(text_align,                    TextAlign),
    COMPUTED_STYLE_MAP_ENTRY(text_decoration,               TextDecoration),
    COMPUTED_STYLE_MAP_ENTRY(text_indent,                   TextIndent),
    // COMPUTED_STYLE_MAP_ENTRY(text_shadow,                TextShadow),
    COMPUTED_STYLE_MAP_ENTRY(text_transform,                TextTransform),
    COMPUTED_STYLE_MAP_ENTRY(top,                           Top),
    COMPUTED_STYLE_MAP_ENTRY(unicode_bidi,                  UnicodeBidi),
    COMPUTED_STYLE_MAP_ENTRY(vertical_align,                VerticalAlign),
    COMPUTED_STYLE_MAP_ENTRY(visibility,                    Visibility),
    // COMPUTED_STYLE_MAP_ENTRY(voice_family,               VoiceFamily),
    // COMPUTED_STYLE_MAP_ENTRY(volume,                     Volume),
    COMPUTED_STYLE_MAP_ENTRY(white_space,                   WhiteSpace),
    // COMPUTED_STYLE_MAP_ENTRY(widows,                     Widows),
    COMPUTED_STYLE_MAP_ENTRY(width,                         Width),
    COMPUTED_STYLE_MAP_ENTRY(word_spacing,                  WordSpacing),
    COMPUTED_STYLE_MAP_ENTRY(z_index,                       ZIndex),

    /* ******************************* *\
     * Implementations of -moz- styles *
    \* ******************************* */

    COMPUTED_STYLE_MAP_ENTRY(appearance,                    Appearance),
    COMPUTED_STYLE_MAP_ENTRY(_moz_background_clip,          BackgroundClip),
    COMPUTED_STYLE_MAP_ENTRY(_moz_background_inline_policy, BackgroundInlinePolicy),
    COMPUTED_STYLE_MAP_ENTRY(_moz_background_origin,        BackgroundOrigin),
    COMPUTED_STYLE_MAP_ENTRY(binding,                       Binding),
    COMPUTED_STYLE_MAP_ENTRY(border_bottom_colors,          BorderBottomColors),
    COMPUTED_STYLE_MAP_ENTRY(border_left_colors,            BorderLeftColors),
    COMPUTED_STYLE_MAP_ENTRY(border_right_colors,           BorderRightColors),
    COMPUTED_STYLE_MAP_ENTRY(border_top_colors,             BorderTopColors),
    COMPUTED_STYLE_MAP_ENTRY(_moz_border_radius_bottomLeft, BorderRadiusBottomLeft),
    COMPUTED_STYLE_MAP_ENTRY(_moz_border_radius_bottomRight,BorderRadiusBottomRight),
    COMPUTED_STYLE_MAP_ENTRY(_moz_border_radius_topLeft,    BorderRadiusTopLeft),
    COMPUTED_STYLE_MAP_ENTRY(_moz_border_radius_topRight,   BorderRadiusTopRight),
    COMPUTED_STYLE_MAP_ENTRY(box_align,                     BoxAlign),
    COMPUTED_STYLE_MAP_ENTRY(box_direction,                 BoxDirection),
    COMPUTED_STYLE_MAP_ENTRY(box_flex,                      BoxFlex),
    COMPUTED_STYLE_MAP_ENTRY(box_ordinal_group,             BoxOrdinalGroup),
    COMPUTED_STYLE_MAP_ENTRY(box_orient,                    BoxOrient),
    COMPUTED_STYLE_MAP_ENTRY(box_pack,                      BoxPack),
    COMPUTED_STYLE_MAP_ENTRY(box_sizing,                    BoxSizing),
    COMPUTED_STYLE_MAP_ENTRY(_moz_column_count,             ColumnCount),
    COMPUTED_STYLE_MAP_ENTRY(_moz_column_width,             ColumnWidth),
    COMPUTED_STYLE_MAP_ENTRY(_moz_column_gap,               ColumnGap),
    COMPUTED_STYLE_MAP_ENTRY(float_edge,                    FloatEdge),
    COMPUTED_STYLE_MAP_ENTRY(force_broken_image_icon,  ForceBrokenImageIcon),
    COMPUTED_STYLE_MAP_ENTRY(image_region,                  ImageRegion),
    COMPUTED_STYLE_MAP_ENTRY(_moz_outline_radius_bottomLeft, OutlineRadiusBottomLeft),
    COMPUTED_STYLE_MAP_ENTRY(_moz_outline_radius_bottomRight,OutlineRadiusBottomRight),
    COMPUTED_STYLE_MAP_ENTRY(_moz_outline_radius_topLeft,    OutlineRadiusTopLeft),
    COMPUTED_STYLE_MAP_ENTRY(_moz_outline_radius_topRight,   OutlineRadiusTopRight),
    COMPUTED_STYLE_MAP_ENTRY(user_focus,                    UserFocus),
    COMPUTED_STYLE_MAP_ENTRY(user_input,                    UserInput),
    COMPUTED_STYLE_MAP_ENTRY(user_modify,                   UserModify),
    COMPUTED_STYLE_MAP_ENTRY(user_select,                   UserSelect)

#ifdef MOZ_SVG
    ,
    COMPUTED_STYLE_MAP_ENTRY(clip_path,                     ClipPath),
    COMPUTED_STYLE_MAP_ENTRY(clip_rule,                     ClipRule),
    COMPUTED_STYLE_MAP_ENTRY(color_interpolation,           ColorInterpolation),
    COMPUTED_STYLE_MAP_ENTRY(color_interpolation_filters,   ColorInterpolationFilters),
    COMPUTED_STYLE_MAP_ENTRY(dominant_baseline,             DominantBaseline),
    COMPUTED_STYLE_MAP_ENTRY(fill,                          Fill),
    COMPUTED_STYLE_MAP_ENTRY(fill_opacity,                  FillOpacity),
    COMPUTED_STYLE_MAP_ENTRY(fill_rule,                     FillRule),
    COMPUTED_STYLE_MAP_ENTRY(filter,                        Filter),
    COMPUTED_STYLE_MAP_ENTRY(flood_color,                   FloodColor),
    COMPUTED_STYLE_MAP_ENTRY(flood_opacity,                 FloodOpacity),
    COMPUTED_STYLE_MAP_ENTRY(lighting_color,                LightingColor),
    COMPUTED_STYLE_MAP_ENTRY(mask,                          Mask),
    COMPUTED_STYLE_MAP_ENTRY(marker_end,                    MarkerEnd),
    COMPUTED_STYLE_MAP_ENTRY(marker_mid,                    MarkerMid),
    COMPUTED_STYLE_MAP_ENTRY(marker_start,                  MarkerStart),
    COMPUTED_STYLE_MAP_ENTRY(pointer_events,                PointerEvents),
    COMPUTED_STYLE_MAP_ENTRY(shape_rendering,               ShapeRendering),
    COMPUTED_STYLE_MAP_ENTRY(stop_color,                    StopColor),
    COMPUTED_STYLE_MAP_ENTRY(stop_opacity,                  StopOpacity),
    COMPUTED_STYLE_MAP_ENTRY(stroke,                        Stroke),
    COMPUTED_STYLE_MAP_ENTRY(stroke_dasharray,              StrokeDasharray),
    COMPUTED_STYLE_MAP_ENTRY(stroke_dashoffset,             StrokeDashoffset),
    COMPUTED_STYLE_MAP_ENTRY(stroke_linecap,                StrokeLinecap),
    COMPUTED_STYLE_MAP_ENTRY(stroke_linejoin,               StrokeLinejoin),
    COMPUTED_STYLE_MAP_ENTRY(stroke_miterlimit,             StrokeMiterlimit),
    COMPUTED_STYLE_MAP_ENTRY(stroke_opacity,                StrokeOpacity),
    COMPUTED_STYLE_MAP_ENTRY(stroke_width,                  StrokeWidth),
    COMPUTED_STYLE_MAP_ENTRY(text_anchor,                   TextAnchor),
    COMPUTED_STYLE_MAP_ENTRY(text_rendering,                TextRendering)
#endif

  };

  *aLength = NS_ARRAY_LENGTH(map);

  return map;
}

